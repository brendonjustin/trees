#ifndef trees_foreset_h
#define trees_foreset_h

#include <vector>

class Forest
{
 public:
  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Forest(ArgParser *a) : args(a), treeSize(0) { };
  ~Forest();

  // ===+=====
  // RENDERING
  void initializeVBOs();
  void setupVBOs();
  void drawVBOs();
  void cleanupVBOs();

 private:
  // helper functions
  
  // ==============
  // REPRESENTATION
  ArgParser *args;
  //Each of the following are to allow for multiple textures in a model
  //vertices[0], edges[0], and triangles[0] are all of the data for better access
  std::vector<std::vector<Vertex*> > vertices;
  std::vector<edgeshashtype> edges;
  std::vector<Material*> materials;
  std::vector<triangleshashtype> triangles;

  std::vector<GLuint> forest_tri_verts_VBO;
  std::vector<GLuint> forest_tri_indices_VBO;
  std::vector<GLuint> forest_tri_texcoords_VBO;

  std::vector<std::vector<float> > terrainHeights;


  //Ground representation
  std::vector<Vertex*> g_vertices;
  edgeshashtype g_edges;
  triangleshashtype g_triangles;
  
  int num_gnd_tris;
  
  GLuint gnd_mesh_tri_verts_VBO;
  GLuint gnd_mesh_tri_indices_VBO;
  GLuint gnd_mesh_verts_VBO;

  int num_trees;
  int treeSize;
};

#endif
