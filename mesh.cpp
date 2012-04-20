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
#include "argparser.h"

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

Edge* Mesh::getEdge(Vertex *a, Vertex *b) const {
  edgeshashtype::const_iterator iter = edges.find(std::make_pair(a,b));
  if (iter == edges.end()) return NULL;
  return iter->second;
}

// =======================================================================
// the load function parses very simple .obj files
// the basic format has been extended to allow the specification 
// of crease weights on the edges.
// This function needs a COMPLETE rework
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
      Edge *ab = getEdge(va,vb);
      Edge *ba = getEdge(vb,va);
      assert (ab != NULL);
      assert (ba != NULL);
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
  glGenBuffers(1, &gnd_mesh_tri_verts_VBO);
  glGenBuffers(1, &gnd_mesh_tri_indices_VBO);
  glGenBuffers(1, &gnd_mesh_verts_VBO);
  setupVBOs();
}

void Mesh::setupVBOs() {
  HandleGLError("in setup mesh VBOs");
  setupGndTriVBOs();
  setupTriVBOs();
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
        for (unsigned int k = 0; k < normals.size(); k++) {
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

void Mesh::setupGndTriVBOs() {
  //  The scene's ground
  VBOTriVert* gnd_mesh_tri_verts;
  VBOTri* gnd_mesh_tri_indices;
  unsigned int num_tris = 2;
  
  Vec3f a, b, c, d, normal;
  
  gnd_mesh_tri_verts = new VBOTriVert[num_tris*3];
  gnd_mesh_tri_indices = new VBOTri[num_tris];
  
  a = Vec3f(-1, 0, -1);
  b = Vec3f(1,  0,  1);
  c = Vec3f(1,  0, -1);
  d = Vec3f(-1, 0,  1);
  normal = Vec3f(0, 0, 1);
  
  gnd_mesh_tri_verts[0] = VBOTriVert(a,normal);
  gnd_mesh_tri_verts[1] = VBOTriVert(b,normal);
  gnd_mesh_tri_verts[2] = VBOTriVert(c,normal);
  gnd_mesh_tri_verts[3] = VBOTriVert(d,normal);

  gnd_mesh_tri_indices[0] = VBOTri(0, 1, 2);
  gnd_mesh_tri_indices[1] = VBOTri(1, 0, 3);

  glBindBuffer(GL_ARRAY_BUFFER,gnd_mesh_tri_verts_VBO);
  glBufferData(GL_ARRAY_BUFFER,
         sizeof(VBOTriVert) * num_tris * 3,
         gnd_mesh_tri_verts,
         GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,gnd_mesh_tri_indices_VBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
         sizeof(VBOTri) * num_tris,
         gnd_mesh_tri_indices,
         GL_STATIC_DRAW);

  delete [] gnd_mesh_tri_verts;
  delete [] gnd_mesh_tri_indices;
}

void Mesh::cleanupVBOs() {
  glDeleteBuffers(1, &mesh_tri_verts_VBO);
  glDeleteBuffers(1, &mesh_tri_indices_VBO);
  glDeleteBuffers(1, &gnd_mesh_tri_verts_VBO);
  glDeleteBuffers(1, &gnd_mesh_tri_indices_VBO);
  glDeleteBuffers(1, &gnd_mesh_verts_VBO);
}

void Mesh::drawVBOs() {

  HandleGLError("in draw mesh");

  // scale it so it fits in the window
  Vec3f center;
  float s = 1;
  glScalef(s,s,s);
  glTranslatef(-center.x(),-center.y(),-center.z());

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

  // now draw the ground
  glColor3f(0,1,0);
  glBindBuffer(GL_ARRAY_BUFFER, gnd_mesh_tri_verts_VBO);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(0));
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(12));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gnd_mesh_tri_indices_VBO);
  glDrawElements(GL_TRIANGLES,
    4*3,
    GL_UNSIGNED_INT,
    BUFFER_OFFSET(0));

  HandleGLError("leaving draw VBOs");
}
