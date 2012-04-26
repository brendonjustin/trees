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

const float HEMISPHERE_PI = 3.1415926535;

class View;
class Mesh;
class Vec3f;

class Hemisphere
{
 public:
  //Constructors
  Hemisphere();
  Hemisphere(Mesh* inmesh, int inlevels);

  //Destructor
  ~Hemisphere();

  //Accessors
  View* getView(int i) {return view[i];}
  int numViews() {return std::pow(2, levels+1) - 3;}

  //General use functions
  void setup();
  View* getNearestView(float angXZ, float angY);
  View* getInterpolatedView(float angXZ, float angY);
  Vec3f getAngleFromIndex(int index);

 private:
  //The number of levels of points, including the one at the top
  int levels;

  //The mesh that this hemisphere surrounds
  Mesh* mesh;

  //The minimum and maximum bounds of the mesh
  Vec3f min, max;

  //The views themselves
  View** view;

  //Helper functions
  void computeBounds();
  int viewsAtLevel(int l);
};

#endif
