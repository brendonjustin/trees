#ifndef MESH_H
#define MESH_H

#include <vector>
#include "vectors.h"
#include "hash.h"
#include "material.h"

class Vertex;
class Edge;
class Triangle;
class ArgParser;
class Ray;
class Hit;

// ======================================================================
// ======================================================================

// helper structures for VBOs, for rendering
// (note, the data stored in each of these is application specific, 
// adjust as needed!)

struct VBOVert {
  VBOVert() {}
  VBOVert(const Vec3f &p) {
    x = p.x(); y = p.y(); z = p.z();
  }
  float x, y, z;    // position
};

struct VBOEdge {
  VBOEdge() {}
  VBOEdge(unsigned int a, unsigned int b) {
    verts[0] = a;
    verts[1] = b;
  }
  unsigned int verts[2];
};

struct VBOTriVert {
  VBOTriVert() {}
  VBOTriVert(const Vec3f &p, const Vec3f &n) {
    x = p.x(); y = p.y(); z = p.z();
    nx = n.x(); ny = n.y(); nz = n.z();
  }
  float x, y, z;    // position
  float nx, ny, nz; // normal
};

struct VBOTri {
  VBOTri() {}
  VBOTri(unsigned int a, unsigned int b, unsigned int c) {
    verts[0] = a;
    verts[1] = b;
    verts[2] = c;
  }
  unsigned int verts[3];
};

struct VBOTex {
  VBOTex() {}
  VBOTex(float u, float v) : s(u),t(v) {}
  float s,t;
};
  

class Mesh
{
 public:
  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Mesh(ArgParser *a) { args = a; }
  ~Mesh();
  void Load(const std::string &input_file);

  // ========
  // VERTICES
  int numVertices() const {return vertices[0].size();}
  Vertex* addVertex(const Vec3f &pos, int mat = -1);
  // look up vertex by index from original .obj file
  Vertex* getVertex(int i) const {
    assert (i >= 0 && i < numVertices());
    Vertex *v = vertices[0][i];
    assert (v != NULL);
    return v; }

  // =====
  // EDGES
  int numEdges() const { return edges[0].size(); }
  // this efficiently looks for an edge with the given vertices, using a hash table
  Edge* getEdge(Vertex *a, Vertex *b) const;
  const edgeshashtype& getEdges() const { return edges[0]; }

  // =========
  // TRIANGLES
  int numTriangles() const { return triangles[0].size(); }
  int addTriangle(Vertex *a, Vertex *b, Vertex *c, int mat = -1);
  void removeTriangle(Triangle *t, int mat = -1);

  // =====
  // OTHER
  int numMaterials() const {return materials.size();}

  // ===+=====
  // RENDERING
  void initializeVBOs();
  void setupVBOs();
  void drawVBOs(bool view = false);
  void cleanupVBOs();

 private:
  // helper functions
  void setupTriVBOs(int mat);
  void setupGndTriVBOs();
  
  // ==============
  // REPRESENTATION
  ArgParser *args;
  //Each of the following are to allow for multiple textures in a model
  //vertices[0], edges[0], and triangles[0] are all of the data for better access
  std::vector<std::vector<Vertex*> > vertices;
  std::vector<edgeshashtype> edges;
  std::vector<Material*> materials;
  std::vector<triangleshashtype> triangles;

  std::vector<GLuint> mesh_tri_verts_VBO;
  std::vector<GLuint> mesh_tri_indices_VBO;
  std::vector<GLuint> mesh_tri_texcoords_VBO;

  //Ground representation
  std::vector<Vertex*> g_vertices;
  edgeshashtype g_edges;
  triangleshashtype g_triangles;
  
  int num_gnd_tris;
  
  GLuint gnd_mesh_tri_verts_VBO;
  GLuint gnd_mesh_tri_indices_VBO;
  GLuint gnd_mesh_verts_VBO;
  
 public:
  Vec3f background_color;

 private:
};

#endif
