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

#include "seeder.h"
#include "terraingenerator.h"

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
  for (triangleshashtype::iterator iter = triangles[0].begin();
       iter != triangles[0].end(); iter++) {
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
    delete vertices[0][i];
  }
}

// =======================================================================
// MODIFIERS:   ADD & REMOVE
// =======================================================================

Vertex* Mesh::addVertex(const Vec3f &position, int mat) {
  int index = numVertices();
  Vertex *v = new Vertex(index, position);
  vertices[0].push_back(v);
  if (mat != -1) vertices[mat+1].push_back(v);
  return v;
}

int Mesh::addTriangle(Vertex *a, Vertex *b, Vertex *c, int mat) {
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
  assert (edges[0].find(std::make_pair(a,b)) == edges[0].end());
  assert (edges[0].find(std::make_pair(b,c)) == edges[0].end());
  assert (edges[0].find(std::make_pair(c,a)) == edges[0].end());
  // add the edges to the master list
  edges[0][std::make_pair(a,b)] = ea;
  edges[0][std::make_pair(b,c)] = eb;
  edges[0][std::make_pair(c,a)] = ec;
  // connect up with opposite edges (if they exist)
  edgeshashtype::iterator ea_op = edges[0].find(std::make_pair(b,a)); 
  edgeshashtype::iterator eb_op = edges[0].find(std::make_pair(c,b)); 
  edgeshashtype::iterator ec_op = edges[0].find(std::make_pair(a,c)); 
  if (ea_op != edges[0].end()) { ea_op->second->setOpposite(ea); }
  if (eb_op != edges[0].end()) { eb_op->second->setOpposite(eb); }
  if (ec_op != edges[0].end()) { ec_op->second->setOpposite(ec); }
  // add the triangle to the master list
  assert (triangles[0].find(t->getID()) == triangles[0].end());
  triangles[0][t->getID()] = t;
  //Add to specific texture lists
  if (mat != -1)
    {
      mat++;
      edges[mat][std::make_pair(a,b)] = ea;
      edges[mat][std::make_pair(b,c)] = eb;
      edges[mat][std::make_pair(c,a)] = ec;
      triangles[mat][t->getID()] = t;
    }
  
  return t->getID();
}


void Mesh::removeTriangle(Triangle *t, int mat) {
  Edge *ea = t->getEdge();
  Edge *eb = ea->getNext();
  Edge *ec = eb->getNext();
  Vertex *a = ea->getStartVertex();
  Vertex *b = eb->getStartVertex();
  Vertex *c = ec->getStartVertex();
  // remove these elements from master lists
  edges[0].erase(std::make_pair(a,b)); 
  edges[0].erase(std::make_pair(b,c)); 
  edges[0].erase(std::make_pair(c,a)); 
  triangles[0].erase(t->getID());
  if (mat != -1) triangles[mat+1].erase(t->getID());
  // clean up memory
  delete ea;
  delete eb;
  delete ec;
  delete t;
}

Edge* Mesh::getEdge(Vertex *a, Vertex *b) const {
  edgeshashtype::const_iterator iter = edges[0].find(std::make_pair(a,b));
  if (iter == edges[0].end()) return NULL;
  return iter->second;
}

// =======================================================================
// the load function parses very simple .obj files
// =======================================================================

#define MAX_CHAR_PER_LINE 200

void Mesh::Load(const std::string &input_file) {

  std::ifstream istr(input_file.c_str());
  if (!istr) {
    std::cout << "ERROR! CANNOT OPEN: " << input_file << std::endl;
    return;
  }

  // extract the directory from the .obj filename, to use for texture files
  int last_slash = input_file.rfind("/");
  std::string directory = input_file.substr(0,last_slash+1);

  //Give all mesh vectors their [0]
  vertices.resize(1);
  edges.resize(1);
  materials.clear();
  triangles.resize(1);
  mesh_tri_verts_VBO.resize(0);
  mesh_tri_indices_VBO.resize(0);
  mesh_tri_texcoords_VBO.resize(0);

  char line[MAX_CHAR_PER_LINE];
  std::string token, token2;
  float x,y,z;
  int a,b,c;
  int index = 0;
  int vert_count = 0;
  int vert_index = 1;
  Material *active_material = NULL;
  int mat_index = -1;
  std::vector<std::pair<float, float> > textures;

  // read in each line of the file
  while (istr.getline(line,MAX_CHAR_PER_LINE)) { 
    // put the line into a stringstream for parsing
    std::stringstream ss;
    ss << line;

    // check for blank line
    token = "";   
    ss >> token;
    if (token == "") continue;

    if (token == std::string("g")) {
      vert_index = 1; 
      index++;
    } else if (token == std::string("v")) {
      vert_count++;
      ss >> x >> y >> z;
      addVertex(Vec3f(x,y,z), mat_index);
    } else if (token == std::string("vt")) {
      ss >> x >> y;
      textures.push_back(std::make_pair(x,y));
    } else if (token == std::string("f")) {
      a = b = c = -1;
      ss >> a >> b >> c;
      a -= vert_index;
      b -= vert_index;
      c -= vert_index;
      assert (a >= 0 && a < numVertices());
      assert (b >= 0 && b < numVertices());
      assert (c >= 0 && c < numVertices());
      int id = addTriangle(getVertex(a),getVertex(b),getVertex(c), mat_index);
      triangles[0][id]->setTextureCoordinates(0, textures[a].first, textures[a].second);
      triangles[0][id]->setTextureCoordinates(1, textures[b].first, textures[b].second);
      triangles[0][id]->setTextureCoordinates(2, textures[c].first, textures[c].second);
      if (active_material != NULL) triangles[0][id]->setMaterial(active_material);
    } else if (token == "m") {
      // this is not standard .obj format!!
      // materials
      int m;
      ss >> m;
      assert (m >= 0 && m < (int)materials.size());
      active_material = materials[m];
      mat_index = m;
    } else if (token == "material") {
      // this is not standard .obj format!!
      std::string texture_file = "";

      //Texture
      istr.getline(line,MAX_CHAR_PER_LINE);
      std::stringstream ss2;
      ss2 << line;
      ss2 >> token;
      assert (token == "map_Kd");
      ss2 >> texture_file;
      // prepend the directory name
      texture_file = directory + texture_file;
      
      materials.push_back(new Material(texture_file,Vec3f(1,1,1),Vec3f(0,0,0),Vec3f(0,0,0),0));
      vertices.push_back(std::vector<Vertex*>());
      edges.push_back(edgeshashtype());
      triangles.push_back(triangleshashtype());
      mesh_tri_verts_VBO.push_back(0);
      mesh_tri_indices_VBO.push_back(0);
      mesh_tri_texcoords_VBO.push_back(0);
      
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
  glGenBuffers(numMaterials(), &mesh_tri_verts_VBO[0]);
  glGenBuffers(numMaterials(), &mesh_tri_indices_VBO[0]);
  glGenBuffers(numMaterials(), &mesh_tri_texcoords_VBO[0]);
  glGenBuffers(1, &gnd_mesh_tri_verts_VBO);
  glGenBuffers(1, &gnd_mesh_tri_indices_VBO);
  glGenBuffers(1, &gnd_mesh_verts_VBO);
  setupVBOs();
}

void Mesh::setupVBOs() {
  HandleGLError("in setup mesh VBOs");
  setupGndTriVBOs();
  for (int i = 0; i < numMaterials(); i++)
    {
      setupTriVBOs(i);
    }
  HandleGLError("leaving setup mesh");
}

void Mesh::setupTriVBOs(int mat) {
  VBOTriVert* mesh_tri_verts;
  VBOTri* mesh_tri_indices;
  VBOTex* mesh_tri_texcoords;
  unsigned int num_tris = triangles[mat+1].size();
  
  // allocate space for the data
  mesh_tri_verts = new VBOTriVert[num_tris*3];
  mesh_tri_indices = new VBOTri[num_tris];
  mesh_tri_texcoords = new VBOTex[num_tris*3];

  // write the vertex & triangle data
  unsigned int i = 0;
  triangleshashtype::iterator iter = triangles[mat+1].begin();
  for (; iter != triangles[mat].end(); iter++,i++) {
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
    mesh_tri_texcoords[i*3] = VBOTex(t->get_s(0), t->get_t(0));
    mesh_tri_texcoords[i*3+1] = VBOTex(t->get_s(1), t->get_t(1));
    mesh_tri_texcoords[i*3+2] = VBOTex(t->get_s(2), t->get_t(2));
  }

  // cleanup old buffer data (if any)
  glDeleteBuffers(1, (&mesh_tri_verts_VBO[mat]));
  glDeleteBuffers(1, (&mesh_tri_indices_VBO[mat]));
  glDeleteBuffers(1, (&mesh_tri_texcoords_VBO[mat]));

  
  // copy the data to each VBO
  glBindBuffer(GL_ARRAY_BUFFER,mesh_tri_verts_VBO[mat]); 
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOTriVert) * num_tris * 3,
	       mesh_tri_verts,
	       GL_STATIC_DRAW); 
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mesh_tri_indices_VBO[mat]); 
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
	       sizeof(VBOTri) * num_tris,
	       mesh_tri_indices, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER,mesh_tri_texcoords_VBO[mat]);
  HandleGLError("Before setting up VBOs");
  
  glBufferData(GL_ARRAY_BUFFER,
	       sizeof(VBOTex) * num_tris * 3,
	       mesh_tri_texcoords,
	       GL_STATIC_DRAW);
  
  HandleGLError("After setting up VBOs");

  delete [] mesh_tri_verts;
  delete [] mesh_tri_indices;
  delete [] mesh_tri_texcoords;

}

void Mesh::setupGndTriVBOs() {
  //  The scene's ground
  bool genTerrain = true;
  bool genTreeDist = false;
  int numBlocks, numTrees = 0;
  float area;
  Vec3f a, b, c, d, normal;
  VBOTriVert* gnd_mesh_tri_verts;
  VBOTri* gnd_mesh_tri_indices;
  
  //  Note: numBlocks must be a power of two, squared, i.e. (2^x)^2
  //  Area need not be an integer value
  numBlocks = pow(pow(2, 7), 2);
  area = numBlocks;
  
  normal = Vec3f(0, 1, 0);
  
  //  the rest of the function is code to generate ground terrain,
  //  or distribute trees according to generated locations
  if (genTerrain) {
    std::vector<std::vector<float> > heights;
    Vec3f baseOffset, offset, hVec;
    float sideLength = sqrt(area / numBlocks);
    
    //  Tweak some optional parameters
//    TerrainGenerator::setRatio(0.5f);
//    TerrainGenerator::setRatio(1.5f);
    TerrainGenerator::setRatio(2.0f);
//    TerrainGenerator::setRatio(2.5f);
//    TerrainGenerator::setScale(2.0f);
    TerrainGenerator::setScale(100.0f);
    heights = TerrainGenerator::generate((int)sqrt(numBlocks));
    
    //  A variable sized square with the bottom left corner at 0,0
    a = Vec3f(0,            0,  0);
    b = Vec3f(sideLength,   0,  sideLength);
    c = Vec3f(sideLength,   0,  0);
    d = Vec3f(0,            0,  sideLength);
    hVec = Vec3f(0,1,0);
    
    gnd_mesh_tri_verts = new VBOTriVert[numBlocks*4];
    gnd_mesh_tri_indices = new VBOTri[numBlocks*2];
    
    //  Draw ground squares
    int locCounter = 0;
    for (int i = 0; i < sqrt(numBlocks); ++i) {
      baseOffset = c*i;
      for (int j = 0; j < sqrt(numBlocks); ++j) {
        offset = baseOffset + d*j;
        gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + a + hVec*heights[i][j], normal);
        gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + b + hVec*heights[i+1][j+1], normal);
        gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + c + hVec*heights[i+1][j], normal);
        gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + d + hVec*heights[i][j+1], normal);
        
        gnd_mesh_tri_indices[locCounter / 2 - 2] = VBOTri(locCounter - 4, locCounter - 3, locCounter - 2);
        gnd_mesh_tri_indices[locCounter / 2 - 1] = VBOTri(locCounter - 3, locCounter - 4, locCounter - 1);
      }
    }
    
    glBindBuffer(GL_ARRAY_BUFFER,gnd_mesh_tri_verts_VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(VBOTriVert) * numBlocks * 4,
                 gnd_mesh_tri_verts,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,gnd_mesh_tri_indices_VBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(VBOTri) * numBlocks * 2,
                 gnd_mesh_tri_indices,
                 GL_STATIC_DRAW);
    
    num_gnd_tris = numBlocks * 2;
    
    delete [] gnd_mesh_tri_verts;
    delete [] gnd_mesh_tri_indices;
  } else if (genTreeDist) {
    Seeder seeder = Seeder(2);
    std::vector<Vec3f> locations;
    
    //  A 1x1 square with the bottom left corner at 0,0
    a = Vec3f(0,  0,  0);
    b = Vec3f(1,  0,  1);
    c = Vec3f(1,  0,  0);
    d = Vec3f(0,  0,  1);
    
    locations = seeder.getTreeLocations(area, numBlocks);
    
    numTrees = locations.size();
    
    gnd_mesh_tri_verts = new VBOTriVert[numTrees*4];
    gnd_mesh_tri_indices = new VBOTri[numTrees*2];
    
    //  Draw ground squares
    int locCounter = 0;
    for (int i = 0; i < numTrees; ++i) {
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(locations[i]+a, normal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(locations[i]+b, normal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(locations[i]+c, normal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(locations[i]+d, normal);
      
      gnd_mesh_tri_indices[locCounter / 2 - 2] = VBOTri(locCounter - 4, locCounter - 3, locCounter - 2);
      gnd_mesh_tri_indices[locCounter / 2 - 1] = VBOTri(locCounter - 3, locCounter - 4, locCounter - 1);
    }
    
    glBindBuffer(GL_ARRAY_BUFFER,gnd_mesh_tri_verts_VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(VBOTriVert) * numTrees * 4,
                 gnd_mesh_tri_verts,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,gnd_mesh_tri_indices_VBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(VBOTri) * numTrees * 2,
                 gnd_mesh_tri_indices,
                 GL_STATIC_DRAW);
    
    num_gnd_tris = numTrees * 2;
    
    delete [] gnd_mesh_tri_verts;
    delete [] gnd_mesh_tri_indices;
  }
}

void Mesh::cleanupVBOs() {
  glDeleteBuffers(numMaterials(), &mesh_tri_verts_VBO[0]);
  glDeleteBuffers(numMaterials(), &mesh_tri_indices_VBO[0]);
  glDeleteBuffers(numMaterials(), &mesh_tri_texcoords_VBO[0]);
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

  //No lighting for rendering views
  glDisable(GL_LIGHTING);
  glColor3f(1.0,1.0,1.0);

  // ======================
  // draw all the triangles
  for (int i = 0; i < numMaterials(); i++)
    {
      unsigned int num_tris = triangles[i+1].size();
      
      HandleGLError("Right before texture");
      
      //Select the texture to use
      int a;
      glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, &a);
      glBindTexture(GL_TEXTURE_2D, materials[i]->getTextureID());
      glColor3f(1,1,1);
      glGetIntegerv(GL_CLIENT_ACTIVE_TEXTURE, &a);
      HandleGLError("Right after texture");

      //Select the texture coordinate buffer
      glBindBuffer(GL_ARRAY_BUFFER, mesh_tri_texcoords_VBO[i]);
      //Enable it
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, BUFFER_OFFSET(0));
      
      // select the vertex buffer
      glBindBuffer(GL_ARRAY_BUFFER, mesh_tri_verts_VBO[i]);
      // describe the layout of data in the vertex buffer
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(0));
      glEnableClientState(GL_NORMAL_ARRAY);
      glNormalPointer(GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(12));

      
      // select the index buffer
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_tri_indices_VBO[i]);
      // draw this data
      glDrawElements(GL_TRIANGLES, 
		     num_tris*3,
		     GL_UNSIGNED_INT,
		     BUFFER_OFFSET(0));
      
      glDisableClientState(GL_NORMAL_ARRAY);
      glDisableClientState(GL_VERTEX_ARRAY);
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

  HandleGLError("leaving draw VBOs");
}

void Mesh::drawGndVBOs() {
  // draw the ground
  HandleGLError("Before drawing ground");
  glEnable(GL_LIGHTING);
  glColor3f(0,1,0);
  glBindBuffer(GL_ARRAY_BUFFER, gnd_mesh_tri_verts_VBO);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(0));
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(12));
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gnd_mesh_tri_indices_VBO);
  glDrawElements(GL_TRIANGLES,
                 num_gnd_tris*3,
                 GL_UNSIGNED_INT,
                 BUFFER_OFFSET(0));
  
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}
