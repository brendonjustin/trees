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
  int numVertices() const { return vertices.size(); }
  Vertex* addVertex(const Vec3f &pos);
  // look up vertex by index from original .obj file
  Vertex* getVertex(int i) const {
    assert (i >= 0 && i < numVertices());
    Vertex *v = vertices[i];
    assert (v != NULL);
    return v; }

  // =====
  // EDGES
  int numEdges() const { return edges.size(); }
  // this efficiently looks for an edge with the given vertices, using a hash table
  Edge* getEdge(Vertex *a, Vertex *b) const;
  const edgeshashtype& getEdges() const { return edges; }

  // =========
  // TRIANGLES
  int numTriangles() const { return triangles.size(); }
  void addTriangle(Vertex *a, Vertex *b, Vertex *c);
  void removeTriangle(Triangle *t);

  // ===+=====
  // RENDERING
  void initializeVBOs();
  void setupVBOs();
  void drawVBOs();
  void cleanupVBOs();

 private:
  // helper functions
  void setupTriVBOs();
  void setupGndTriVBOs();
  
  // ==============
  // REPRESENTATION
  ArgParser *args;
  std::vector<Vertex*> vertices;
  edgeshashtype edges;
  triangleshashtype triangles;
  std::vector<Vertex*> g_vertices;
  edgeshashtype g_edges;
  triangleshashtype g_triangles;

  GLuint mesh_tri_verts_VBO;
  GLuint mesh_tri_indices_VBO;
  GLuint gnd_mesh_tri_verts_VBO;
  GLuint gnd_mesh_tri_indices_VBO;
  GLuint gnd_mesh_verts_VBO;
 public:
  std::vector<Material*> materials;
  Vec3f background_color;
 private:
};

#endif
