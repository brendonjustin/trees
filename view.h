/*
  -----Tree View Class Header-----
  Auston Sterling
  Brendon Justin

  A class for storing a particular view of a tree as a 2D image.
  Contains some extra information per texel to assist in interpolation.
*/

#ifndef _VIEW_H_
#define _VIEW_H_

const int VIEW_SIZE = 256;

#include "vectors.h"
#include "mesh.h"
#include "camera.h"
#include "hit.h"

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
  Vec3f color(int i, int j) {return data[i][j].color;}

  //General use functions
  void computeView(float angXZ, float angY, int distance);

 private:
  //The array of texels
  texel data[VIEW_SIZE][VIEW_SIZE];

  //The point where the tree rests on the ground
  int basex;
  int basey;

  //A pointer to the mesh this is a view of
  Mesh* mesh;
};

#endif
