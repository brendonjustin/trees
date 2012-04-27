/*
  -----Hemishpere Class Declaration-----
  Auston Sterling
  Brendon Justin

  A class for storing and managing the views of a mesh.
*/

#ifndef _HEMI_H_
#define _HEMI_H_

#include "vectors.h"

#include <cmath>
#include <cfloat>
#include <vector>

const float HEMISPHERE_PI = 3.1415926535;

class View;
class Mesh;
class Vec3f;

class Hemisphere
{
 public:
  //Constructors
  Hemisphere();
  Hemisphere(Mesh* inmesh, int inlevels, int inpoints);

  //Destructor
  ~Hemisphere();

  //Accessors
  View* getView(int i, int j) {return view[i][j];}
  int numViews();

  //General use functions
  void setup();
  View* getNearestView(float angXZ, float angY);
  View* getNearestView(Vec3f pos, Vec3f camera);
  View* getInterpolatedView(float angXZ, float angY);

 private:
  //The number of levels of points, including the one at the top
  int levels;

  //The number of points around the base of the hemisphere
  int basepoints;

  //The mesh that this hemisphere surrounds
  Mesh* mesh;

  //The minimum and maximum bounds of the mesh
  Vec3f min, max;

  //The views themselves
  std::vector<std::vector<View*> > view;

  //Helper functions
  void computeBounds();
};

#endif
