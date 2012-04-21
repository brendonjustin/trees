/*
  -----Tree View Class Header-----
  Auston Sterling
  Brendon Justin

  A class for storing a particular view of a tree as a 2D image.
  Contains some extra information per texel to assist in interpolation.
*/

#ifndef _VIEW_H_
#define _VIEW_H_

const int NUM_VIEWS = 253;

#include "vectors.h"
#include "mesh.h"
#include "camera.h"

//Struct for each texel
struct texel
{
  //The color at this point
  Vec3f color;

  //The minimum and maximum depths
  float mind, maxd;

  //The ambient occlusion of this point
  Vec3f occlusion;

  //The opacity of this point
  float opacity;
};

//The view itself
class View
{
 public:
  //Constructors
  View();
  View(Mesh* inmesh);

  //General use functions
  void computeView();

 private:
  //The array of texels
  texel data[NUM_VIEWS];

  //A pointer to the mesh this is a view of
  Mesh* mesh;
};

#endif
