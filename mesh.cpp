#include "glCanvas.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <list>
#include <map>

#include "mesh.h"
#include "edge.h"
#include "vertex.h"
#include "triangle.h"


int Triangle::next_triangle_id = 0;

// helper for VBOs
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

bool ltEdge (const Edge* edge1, const Edge* edge2)
{
  return edge1->Length() < edge2->Length();
}

// =======================================================================
// MESH DESTRUCTOR 
// =======================================================================

Mesh::~Mesh() {
  cleanupVBOs();

  // delete all the triangles
  std::vector<Triangle*> todo;
  for (triangleshashtype::iterator iter = triangles.begin();
       iter != triangles.end(); iter++) {
    Triangle *t = iter->second;
    todo.push_back(t);
  }
  int num_triangles = todo.size();
  for (int i = 0; i < num_triangles; i++) {
    removeTriangle(todo[i]);
  }
  // delete all the vertices
  int num_vertices = numVertices();
  for (int i = 0; i < num_vertices; i++) {
    delete vertices[i];
  }
}

// =======================================================================
// MODIFIERS:   ADD & REMOVE
// =======================================================================

Vertex* Mesh::addVertex(const Vec3f &position) {
  int index = numVertices();
  Vertex *v = new Vertex(index, position);
  vertices.push_back(v);
  if (numVertices() == 1)
    bbox = BoundingBox(position,position);
  else 
    bbox.Extend(position);
  return v;
}


void Mesh::addTriangle(Vertex *a, Vertex *b, Vertex *c) {
  // create the triangle
  Triangle *t = new Triangle();
  // create the edges
  Edge *ea = new Edge(a,b,t);
  Edge *eb = new Edge(b,c,t);
  Edge *ec = new Edge(c,a,t);
  // point the triangle to one of its edges
  t->setEdge(ea);
  // connect the edges to each other
  ea->setNext(eb);
  eb->setNext(ec);
  ec->setNext(ea);
  // verify these edges aren't already in the mesh 
  // (which would be a bug, or a non-manifold mesh)
  assert (edges.find(std::make_pair(a,b)) == edges.end());
  assert (edges.find(std::make_pair(b,c)) == edges.end());
  assert (edges.find(std::make_pair(c,a)) == edges.end());
  // add the edges to the master list
  edges[std::make_pair(a,b)] = ea;
  edges[std::make_pair(b,c)] = eb;
  edges[std::make_pair(c,a)] = ec;
  // connect up with opposite edges (if they exist)
  edgeshashtype::iterator ea_op = edges.find(std::make_pair(b,a)); 
  edgeshashtype::iterator eb_op = edges.find(std::make_pair(c,b)); 
  edgeshashtype::iterator ec_op = edges.find(std::make_pair(a,c)); 
  if (ea_op != edges.end()) { ea_op->second->setOpposite(ea); }
  if (eb_op != edges.end()) { eb_op->second->setOpposite(eb); }
  if (ec_op != edges.end()) { ec_op->second->setOpposite(ec); }
  // add the triangle to the master list
  assert (triangles.find(t->getID()) == triangles.end());
  triangles[t->getID()] = t;
}


void Mesh::removeTriangle(Triangle *t) {
  Edge *ea = t->getEdge();
  Edge *eb = ea->getNext();
  Edge *ec = eb->getNext();
  Vertex *a = ea->getStartVertex();
  Vertex *b = eb->getStartVertex();
  Vertex *c = ec->getStartVertex();
  // remove these elements from master lists
  edges.erase(std::make_pair(a,b)); 
  edges.erase(std::make_pair(b,c)); 
  edges.erase(std::make_pair(c,a)); 
  triangles.erase(t->getID());
  // clean up memory
  delete ea;
  delete eb;
  delete ec;
  delete t;
}


// =======================================================================
// Helper functions for accessing data in the hash table
// =======================================================================

Edge* Mesh::getMeshEdge(Vertex *a, Vertex *b) const {
  edgeshashtype::const_iterator iter = edges.find(std::make_pair(a,b));
  if (iter == edges.end()) return NULL;
  return iter->second;
}

Vertex* Mesh::getChildVertex(Vertex *p1, Vertex *p2) const {
  vphashtype::const_iterator iter = vertex_parents.find(std::make_pair(p1,p2)); 
  if (iter == vertex_parents.end()) return NULL;
  return iter->second; 
}

void Mesh::setParentsChild(Vertex *p1, Vertex *p2, Vertex *child) {
  assert (vertex_parents.find(std::make_pair(p1,p2)) == vertex_parents.end());
  vertex_parents[std::make_pair(p1,p2)] = child; 
}


// =======================================================================
// the load function parses very simple .obj files
// the basic format has been extended to allow the specification 
// of crease weights on the edges.
// =======================================================================

#define MAX_CHAR_PER_LINE 200

void Mesh::Load(const std::string &input_file) {

  std::ifstream istr(input_file.c_str());
  if (!istr) {
    std::cout << "ERROR! CANNOT OPEN: " << input_file << std::endl;
    return;
  }

  char line[MAX_CHAR_PER_LINE];
  std::string token, token2;
  float x,y,z;
  int a,b,c;
  int index = 0;
  int vert_count = 0;
  int vert_index = 1;

  // read in each line of the file
  while (istr.getline(line,MAX_CHAR_PER_LINE)) { 
    // put the line into a stringstream for parsing
    std::stringstream ss;
    ss << line;

    // check for blank line
    token = "";   
    ss >> token;
    if (token == "") continue;

    if (token == std::string("usemtl") ||
	token == std::string("g")) {
      vert_index = 1; 
      index++;
    } else if (token == std::string("v")) {
      vert_count++;
      ss >> x >> y >> z;
      addVertex(Vec3f(x,y,z));
    } else if (token == std::string("f")) {
      a = b = c = -1;
      ss >> a >> b >> c;
      a -= vert_index;
      b -= vert_index;
      c -= vert_index;
      assert (a >= 0 && a < numVertices());
      assert (b >= 0 && b < numVertices());
      assert (c >= 0 && c < numVertices());
      addTriangle(getVertex(a),getVertex(b),getVertex(c));
    } else if (token == std::string("e")) {
      a = b = -1;
      ss >> a >> b >> token2;
      // whoops: inconsistent file format, don't subtract 1
      assert (a >= 0 && a <= numVertices());
      assert (b >= 0 && b <= numVertices());
      if (token2 == std::string("inf")) x = 1000000; // this is close to infinity...
      x = atof(token2.c_str());
      Vertex *va = getVertex(a);
      Vertex *vb = getVertex(b);
      Edge *ab = getMeshEdge(va,vb);
      Edge *ba = getMeshEdge(vb,va);
      assert (ab != NULL);
      assert (ba != NULL);
      ab->setCrease(x);
      ba->setCrease(x);
    } else if (token == std::string("vt")) {
    } else if (token == std::string("vn")) {
    } else if (token[0] == '#') {
    } else {
      printf ("LINE: '%s'",line);
    }
  }
}


// =======================================================================
// DRAWING
// =======================================================================

Vec3f ComputeNormal(const Vec3f &p1, const Vec3f &p2, const Vec3f &p3) {
  Vec3f v12 = p2;
  v12 -= p1;
  Vec3f v23 = p3;
  v23 -= p2;
  Vec3f normal;
  Vec3f::Cross3(normal,v12,v23);
  normal.Normalize();
  return normal;
}


void Mesh::initializeVBOs() {
  // create a pointer for the vertex & index VBOs
  glGenBuffers(1, &mesh_tri_verts_VBO);
  glGenBuffers(1, &mesh_tri_indices_VBO);
  glGenBuffers(1, &mesh_verts_VBO);
  glGenBuffers(1, &mesh_boundary_edge_indices_VBO);
  glGenBuffers(1, &mesh_crease_edge_indices_VBO);
  glGenBuffers(1, &mesh_other_edge_indices_VBO);
  setupVBOs();
}

void Mesh::setupVBOs() {
  HandleGLError("in setup mesh VBOs");
  setupTriVBOs();
  setupEdgeVBOs();
  HandleGLError("leaving setup mesh");
}


void Mesh::setupTriVBOs() {

  VBOTriVert* mesh_tri_verts;
  VBOTri* mesh_tri_indices;
  unsigned int num_tris = triangles.size();

  // allocate space for the data
  mesh_tri_verts = new VBOTriVert[num_tris*3];
  mesh_tri_indices = new VBOTri[num_tris];

  // write the vertex & triangle data
  unsigned int i = 0;
  triangleshashtype::iterator iter = triangles.begin();
  for (; iter != triangles.end(); iter++,i++) {
    Triangle *t = iter->second;
    Vec3f a = (*t)[0]->getPos();
    Vec3f b = (*t)[1]->getPos();
    Vec3f c = (*t)[2]->getPos();
    
    if (args->gouraud) {


      // =====================================
      // ASSIGNMENT: reimplement 
      // =====================================
      Edge *e = t->getEdge();
      Triangle *triangle = t;
      std::vector<Vec3f> normals;
      
      //  Iterate on all three vertices
      for (int j = 0; j < 3; ++j) {
        normals.clear();
        
        //  Iterate on all triangles surrounding the vertex
        do {
          Vec3f pt1 = (*triangle)[0]->getPos();
          Vec3f pt2 = (*triangle)[1]->getPos();
          Vec3f pt3 = (*triangle)[2]->getPos();
          
          normals.push_back(ComputeNormal(pt1, pt2, pt3));
          
          e = e->getNext()->getOpposite();
          triangle = e->getTriangle();
        } while (triangle != t);
        
        Vec3f normal = normals[0];
        for (int k = 0; k < normals.size(); k++) {
          normal += normals[k];
        }
        normal.Normalize();
        
        mesh_tri_verts[i*3 + j] = VBOTriVert(e->getEndVertex()->getPos(), normal);
        
        e = e->getNext();
      }
    } else {
      Vec3f normal = ComputeNormal(a,b,c);
      mesh_tri_verts[i*3]   = VBOTriVert(a,normal);
      mesh_tri_verts[i*3+1] = VBOTriVert(b,normal);
      mesh_tri_verts[i*3+2] = VBOTriVert(c,normal);
    }
    mesh_tri_indices[i] = VBOTri(i*3,i*3+1,i*3+2);
  }

  // cleanup old buffer data (if any)
  glDeleteBuffers(1, &mesh_tri_verts_VBO);
  glDeleteBuffers(1, &mesh_tri_indices_VBO);

  // copy the data to each VBO
  glBindBuffer(GL_ARRAY_BUFFER,mesh_tri_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOTriVert) * num_tris * 3,
	       mesh_tri_verts,
	       GL_STATIC_DRAW); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_tri_indices_VBO); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(VBOTri) * num_tris,
	       mesh_tri_indices, GL_STATIC_DRAW);

  delete [] mesh_tri_verts;
  delete [] mesh_tri_indices;

}


void Mesh::setupEdgeVBOs() {

  VBOVert* mesh_verts;
  VBOEdge* mesh_boundary_edge_indices;
  VBOEdge* mesh_crease_edge_indices;
  VBOEdge* mesh_other_edge_indices;

  mesh_boundary_edge_indices = NULL;
  mesh_crease_edge_indices = NULL;
  mesh_other_edge_indices = NULL;

  unsigned int num_verts = vertices.size();

  // first count the edges of each type
  num_boundary_edges = 0;
  num_crease_edges = 0;
  num_other_edges = 0;
  for (edgeshashtype::iterator iter = edges.begin();
       iter != edges.end(); iter++) {
    Edge *e = iter->second;
    int a = e->getStartVertex()->getIndex();
    int b = e->getEndVertex()->getIndex();
    if (e->getOpposite() == NULL) {
      num_boundary_edges++;
    } else {
      if (a < b) continue; // don't double count edges!
      if (e->getCrease() > 0) num_crease_edges++;
      else num_other_edges++;
    }
  }

  // allocate space for the data
  mesh_verts = new VBOVert[num_verts];
  if (num_boundary_edges > 0)
    mesh_boundary_edge_indices = new VBOEdge[num_boundary_edges];
  if (num_crease_edges > 0)
    mesh_crease_edge_indices = new VBOEdge[num_crease_edges];
  if (num_other_edges > 0)
    mesh_other_edge_indices = new VBOEdge[num_other_edges];

  // write the vertex data
  for (unsigned int i = 0; i < num_verts; i++) {
    mesh_verts[i] = VBOVert(vertices[i]->getPos());
  }

  // write the edge data
  int bi = 0;
  int ci = 0;
  int oi = 0; 
  for (edgeshashtype::iterator iter = edges.begin();
       iter != edges.end(); iter++) {
    Edge *e = iter->second;
    int a = e->getStartVertex()->getIndex();
    int b = e->getEndVertex()->getIndex();
    if (e->getOpposite() == NULL) {
      mesh_boundary_edge_indices[bi++] = VBOEdge(a,b);
    } else {
      if (a < b) continue; // don't double count edges!
      if (e->getCrease() > 0) 
	mesh_crease_edge_indices[ci++] = VBOEdge(a,b);
      else 
	mesh_other_edge_indices[oi++] = VBOEdge(a,b);
    }
  }
  assert (bi == num_boundary_edges);
  assert (ci == num_crease_edges);
  assert (oi == num_other_edges);

  // cleanup old buffer data (if any)
  glDeleteBuffers(1, &mesh_verts_VBO);
  glDeleteBuffers(1, &mesh_boundary_edge_indices_VBO);
  glDeleteBuffers(1, &mesh_crease_edge_indices_VBO);
  glDeleteBuffers(1, &mesh_other_edge_indices_VBO);

  // copy the data to each VBO
  glBindBuffer(GL_ARRAY_BUFFER,mesh_verts_VBO); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOVert) * num_verts,
	       mesh_verts,
	       GL_STATIC_DRAW); 

  if (num_boundary_edges > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_boundary_edge_indices_VBO); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sizeof(VBOEdge) * num_boundary_edges,
		 mesh_boundary_edge_indices, GL_STATIC_DRAW);
  }
  if (num_crease_edges > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_crease_edge_indices_VBO); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sizeof(VBOEdge) * num_crease_edges,
		 mesh_crease_edge_indices, GL_STATIC_DRAW);
  }
  if (num_other_edges > 0) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_other_edge_indices_VBO); 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sizeof(VBOEdge) * num_other_edges,
		 mesh_other_edge_indices, GL_STATIC_DRAW);
  }

  delete [] mesh_verts;
  delete [] mesh_boundary_edge_indices;
  delete [] mesh_crease_edge_indices;
  delete [] mesh_other_edge_indices;
}


void Mesh::cleanupVBOs() {
  glDeleteBuffers(1, &mesh_tri_verts_VBO);
  glDeleteBuffers(1, &mesh_tri_indices_VBO);
  glDeleteBuffers(1, &mesh_verts_VBO);
  glDeleteBuffers(1, &mesh_boundary_edge_indices_VBO);
  glDeleteBuffers(1, &mesh_crease_edge_indices_VBO);
  glDeleteBuffers(1, &mesh_other_edge_indices_VBO);
}


void Mesh::drawVBOs() {

  HandleGLError("in draw mesh");

  // scale it so it fits in the window
  Vec3f center; bbox.getCenter(center);
  float s = 1/bbox.maxDim();
  glScalef(s,s,s);
  glTranslatef(-center.x(),-center.y(),-center.z());

  // this offset prevents "z-fighting" bewteen the edges and faces
  // so the edges will always win
  if (args->wireframe) {
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.1,4.0); 
  } 

  // ======================
  // draw all the triangles
  unsigned int num_tris = triangles.size();
  glColor3f(1,1,1);

  // select the vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, mesh_tri_verts_VBO);
  // describe the layout of data in the vertex buffer
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(0));
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(12));

  // select the index buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_tri_indices_VBO);
  // draw this data
  glDrawElements(GL_TRIANGLES, 
		 num_tris*3,
		 GL_UNSIGNED_INT,
		 BUFFER_OFFSET(0));

  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  if (args->wireframe) {
    glDisable(GL_POLYGON_OFFSET_FILL); 
  }

  // =================================
  // draw the different types of edges
  if (args->wireframe) {
    glDisable(GL_LIGHTING);

    // select the vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, mesh_verts_VBO);
    // describe the layout of data in the vertex buffer
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VBOVert), BUFFER_OFFSET(0));

    // draw all the boundary edges
    glLineWidth(3);
    glColor3f(1,0,0);
    // select the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_boundary_edge_indices_VBO);
    // draw this data
    glDrawElements(GL_LINES, num_boundary_edges*2, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

    // draw all the interior, crease edges
    glLineWidth(3);
    glColor3f(1,1,0);
    // select the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_crease_edge_indices_VBO);
    // draw this data
    glDrawElements(GL_LINES, num_crease_edges*2, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

    // draw all the interior, non-crease edges
    glLineWidth(1);
    glColor3f(0,0,0);
    // select the index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_other_edge_indices_VBO);
    // draw this data
    glDrawElements(GL_LINES, num_other_edges*2, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

    glDisableClientState(GL_VERTEX_ARRAY);
  }

  HandleGLError("leaving draw VBOs");
}

// =================================================================
// SUBDIVISION
// =================================================================


void Mesh::LoopSubdivision() {
  printf ("Subdivide the mesh!\n");
  
  // =====================================
  // ASSIGNMENT: complete this functionality
  // =====================================
  
  std::vector<Edge *> originalEdges, removedEdges;
  std::vector<Vec3f> surroundingVtxPos;
  std::vector<std::pair<Vertex *, Vec3f> > newPositions;
  edgeshashtype::iterator edgesIterator;
  Edge *edge = NULL;
  Edge *e2 = NULL;
  Vertex *v1 = NULL;
  Vertex *v2 = NULL;
  Vec3f v1Pos, v2Pos, v3Pos, v4Pos;
  Vertex *childVtx = NULL;
  Vertex *vtxArray[6];
  Triangle *origTri = NULL;
  Vec3f pos;
  
  int surCount = 0;
  bool skipIteration = false;
  float beta = 0;
  static const float beta3 = 3.0f / 16;
  
  //  For each edge, calculate a new position and add a child vertex
  edgesIterator = edges.begin();
  while (edgesIterator != edges.end()) {
    edge = edgesIterator++->second;
    originalEdges.push_back(edge);
    v1 = edge->getStartVertex();
    v2 = edge->getEndVertex();
    v1Pos = v1->getPos();
    v2Pos = v2->getPos();
    
    e2 = edge;
    if (edge->getCrease() == 0) {
      surroundingVtxPos.clear();
      
      do {
        surroundingVtxPos.push_back(e2->getEndVertex()->getPos());
        e2 = e2->getOpposite();
        if (e2 == NULL) {
          break;
        }
        e2 = e2->getNext();
        if (e2 == NULL) {
          break;
        }
      } while (e2 != edge);
      
      surCount = surroundingVtxPos.size();
      if (surCount > 3) {
        beta = 3.0f / (8 * surCount);
      } else {
        //  Don't know how to handle any number other than 3 vertices
        //  in this case.
        assert(surCount == 3);
        
        beta = beta3;
      }
      
      pos = (1 - surCount*beta) * v1Pos;
      for (int i = 0; i < surCount; ++i) {
        pos += beta * surroundingVtxPos[i];
      }
    } else {
      //  Untested as far as I know
      e2 = edge->getOpposite()->getNext();
      pos = v1Pos * 0.75 + 0.125 * v2Pos + 0.125 * e2->getEndVertex()->getPos();
    }
    newPositions.push_back(std::make_pair(v1, pos));
    
    if (getChildVertex(v1, v2) == NULL) {
      if (edge->getCrease() == 0) {
        v3Pos = edge->getNext()->getEndVertex()->getPos();
        v4Pos = edge->getOpposite()->getNext()->getEndVertex()->getPos();
        
        //  Set the child vertex's positon based on nearby vertices,
        //  weighted by 0.375 for the endpoints and 0.125 for the other
        //  vertices in the triangles formed with the current edge
        pos = 0.375 * v1Pos + 0.375 * v2Pos + 0.125 * v3Pos + 0.125 * v4Pos;
      } else {
        //  Put the vertex at the midpoint of the edge
        pos = 0.5 * v1Pos + 0.5 * v2Pos;
      }
      
      childVtx = this->addVertex(pos);
      this->setParentsChild(v1, v2, childVtx);
    }
  }
  
  //  Set the vertices' new positions
  for (std::vector<std::pair<Vertex *, Vec3f> >::iterator iter = newPositions.begin(); iter != newPositions.end(); iter++) {
    iter->first->setPos(iter->second);
  }
  
  //  Now loop again, removing each triangle and adding 4 triangles
  //  in its place.
  for (int i = 0; i < originalEdges.size(); ++i) {
    skipIteration = false;
    edge = originalEdges[i];
    
    //  Make sure we don't try to subdivide already handled edges
    //  Inefficient but functional.
    for (int j = 0; j < removedEdges.size(); ++j) {
      if (removedEdges[j] == edge) {
        skipIteration = true;
      }
    }
    
    if (skipIteration) {
      continue;
    }
    
    //  vtxArray is in order going around a triangle
    vtxArray[0] = edge->getStartVertex();
    vtxArray[2] = edge->getEndVertex();
    childVtx = getChildVertex(vtxArray[0], vtxArray[2]);
    
    if (childVtx == NULL) {
      continue;
    }
    
    vtxArray[1] = childVtx;
    
    removedEdges.push_back(edge);
    edge = edge->getNext();
    vtxArray[4] = edge->getEndVertex();
    vtxArray[3] = getChildVertex(vtxArray[2], vtxArray[4]);
    vtxArray[5] = getChildVertex(vtxArray[4], vtxArray[0]);
    
    removedEdges.push_back(edge);
    edge = edge->getNext();
    removedEdges.push_back(edge);
    
    //  Return to the first edge
    edge = edge->getNext();
    
    origTri = edge->getTriangle();
    this->removeTriangle(origTri);
    triangles.erase(origTri->getID());
    this->addTriangle(vtxArray[0], vtxArray[1], vtxArray[5]);
    this->addTriangle(vtxArray[1], vtxArray[2], vtxArray[3]);
    this->addTriangle(vtxArray[3], vtxArray[5], vtxArray[1]);
    this->addTriangle(vtxArray[3], vtxArray[4], vtxArray[5]);
  }

}


// =================================================================
// SIMPLIFICATION
// =================================================================

void Mesh::Simplification(int target_tri_count) {
  // clear out any previous relationships between vertices
  vertex_parents.clear();

  printf ("Simplify the mesh! %d -> %d\n", numTriangles(), target_tri_count);

  // =====================================
  // ASSIGNMENT: complete this functionality
  // =====================================
  
  edgeshashtype::iterator edgesIterator = edges.begin();
  std::list<Edge *> smallEdges;
  std::list<Edge *>::iterator smallEdgesIterator;
  std::vector<Vertex *> seenVertices1, seenVertices2;
  std::vector<Vertex *> replaceVertices;
  Edge *edge = NULL;
  Edge *startEdge = NULL;
  Edge *oppEdge = NULL;
  
  while (edgesIterator != edges.end()) {
    smallEdges.push_back(edgesIterator++->second);
  }
  
  //  Sort the edges by length, then iterate over them in sorted order
  smallEdges.sort(ltEdge);
  smallEdgesIterator = smallEdges.begin();
  
  while (numTriangles() > target_tri_count) {
    smallEdgesIterator++;
    if (smallEdgesIterator == smallEdges.end()) {
      break;
    }
    
    seenVertices1.clear();
    seenVertices2.clear();
    replaceVertices.clear();
    edge = *smallEdgesIterator;
    startEdge = edge;
    oppEdge = edge->getOpposite();
    
    //  Count the number of vertices one edge away from the endpoints
    //  of the current edge.
    edge = edge->getNext()->getOpposite();
    while (edge != startEdge) {
      seenVertices1.push_back(edge->getStartVertex());
      edge = edge->getNext()->getOpposite();
    }
    
    startEdge = oppEdge;
    oppEdge = oppEdge->getNext()->getOpposite();
    while (oppEdge != startEdge) {
      seenVertices2.push_back(oppEdge->getStartVertex());
      oppEdge = oppEdge->getNext()->getOpposite();
    }
    
    int matches = 0;
    for (int i = 0; i < seenVertices1.size(); ++i) {
      for (int j = 0; j < seenVertices2.size(); ++j) {
        if (seenVertices1[i] == seenVertices2[j]) {
          ++matches;
        }
      }
    }
    
    //  Expect two matching vertices in the set of vertices surrounding
    //  (but not including) the vertices that make up the edge
    if (matches != 2) {
      continue;
    }
    
    startEdge = edge;
    
    Edge *ea = startEdge;
    Vertex *movedVertex = edge->getStartVertex();
    Vertex *oldEndVertex = edge->getEndVertex();
    std::vector<Triangle *> removalTriangles;
    
    //  Move the start vertex to the average of the start and end 
    //  vertices' positions in the existing edge
    movedVertex->setPos(0.5 * movedVertex->getPos() + 0.5 * oldEndVertex->getPos());
    
    Triangle *tri1 = NULL;
    //  Find which triangles will be deleted
    //  Assume getNext() goes clockwise
    do {
      edge = ea;
      
      //  Save vertices in order
      replaceVertices.push_back(ea->getStartVertex());
      ea = ea->getNext();
      replaceVertices.push_back(ea->getStartVertex());
      ea = ea->getNext();
      replaceVertices.push_back(ea->getStartVertex());
      ea = ea->getNext();
      
      tri1 = edge->getTriangle();
      removalTriangles.push_back(tri1);
      
      ea = edge->getNext()->getOpposite();
    } while (ea != startEdge);
    
    //  Delete said triangles
    for (int i = 0; i < removalTriangles.size(); ++i) {
      //  If an edge will be deleted, remove it from the sorted list
      //  of edges first.
      edge = removalTriangles[i]->getEdge();
      ea = edge;
      ea = ea->getNext();
      smallEdges.remove(edge);
      edge = ea;
      ea = ea->getNext();
      smallEdges.remove(edge);
      edge = ea;
      smallEdges.remove(edge);
      
      this->removeTriangle(removalTriangles[i]);
      triangles.erase(removalTriangles[i]->getID());
    }
    
    //  Make two fewer triangles than were previously in an area.
    //  Note: The first and last vertices are the same vertex.  
    //  Many others are duplicates as well.
    int offset = 0;
    int upperLimit = replaceVertices.size() / 3 - 2;
    for (int i = 0; i < upperLimit; ++i) {
      //  Offset changes to account for removed vertex
      i > 5 ? offset = 1 : offset = 0;
      this->addTriangle(replaceVertices[0], replaceVertices[3*i + 5], replaceVertices[3*i + 2 + offset]);
    }
    
    //  Since we just removed an edge, start at the beginning again
    smallEdgesIterator = smallEdges.begin();
  }
}


// =================================================================
