#ifndef trees_foreset_h
#define trees_foreset_h

#include "argparser.h"
#include "glCanvas.h"
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
  
  std::vector<GLuint> forest_tri_verts_VBO;
  std::vector<GLuint> forest_tri_indices_VBO;
  std::vector<GLuint> forest_tri_texcoords_VBO;

  std::vector<std::vector<float> > terrainHeights;

  //Ground representation
  int num_gnd_tris;
  
  GLuint gnd_mesh_tri_verts_VBO;
  GLuint gnd_mesh_tri_indices_VBO;
  GLuint gnd_mesh_verts_VBO;

  int num_trees;
  int treeSize;
};

#endif
