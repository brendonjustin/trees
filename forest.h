#ifndef trees_foreset_h
#define trees_foreset_h

#include "argparser.h"
#include "glCanvas.h"
#include <vector>

class Hemisphere;

struct VBOTriVert;

class Forest
{
 public:
  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Forest(ArgParser *a, Hemisphere* h);
  ~Forest();

  // ===+=====
  // RENDERING
  void initializeVBOs();
  void setupVBOs();
  void drawVBOs();
  void cleanupVBOs();
  
  // CAMERA ADJUSTMENTS
  void setCameraPosition(Vec3f cameraPos);
  void cameraMoved(Vec3f cameraPos);
  
  void setTreeQuads();

 private:
  // helper functions
  
  // ==============
  // REPRESENTATION
  ArgParser *args;

  Hemisphere *hemisphere;

  float area;
  int num_blocks;
  int num_gnd_tris;
  int num_trees;
  int tree_size;
  bool tree_buffer_set;

  Vec3f camera_pos;
  Vec3f aT, bT, cT, dT;

  //  Hold the world-space coordinates of each tree
  std::vector<std::vector<Vec3f> > tree_locations;
  
  VBOTriVert* forest_quad_verts;
  
  std::vector<GLuint> forest_quad_verts_VBO;
  std::vector<GLuint> forest_quad_indices_VBO;
  std::vector<GLuint> forest_quad_texcoords_VBO;
  std::vector<GLuint> forest_quad_textures;

  //Ground representation
  GLuint gnd_mesh_tri_verts_VBO;
  GLuint gnd_mesh_tri_indices_VBO;
  GLuint gnd_mesh_verts_VBO;
};

#endif
