/*
  -----Octree Class Declaration-----
  Auston Sterling
  Brendon Justin

  An octree for storing triangles for quick access during raytracing.
*/

#ifndef _OCTREE_H_
#define _OCTREE_H_

#include "triangle.h"
#include "vectors.h"
#include "ray.h"

#include <set>

class Octree
{
 public:
  //Constructors
  Octree();
  Octree(Vec3f inmin, Vec3f inmax, unsigned short inmaxTris);

  //Destructor
  ~Octree();

  //General use functions
  bool addTriangle(Triangle* tri);
  void split();
  std::set<Triangle*> getTris();
  std::set<Triangle*> getTris(const Ray & r);
  //void condense(); //This could be added if the octree's triangles can be removed

 private:
  //The minimum and maximum corners of the box
  Vec3f min, max;

  //The eight children of this node
  Octree* child[8];

  //The maximum triangles per node
  unsigned short maxTris;

  //A vector containing pointers to the triangles
  std::set<Triangle*> triangles;
};

#endif
