/*
  -----Hemishpere Class Implementation-----
  Auston Sterling
  Brendon Justin

  A class for storing and managing the views of a mesh.
*/

#ifndef _HEMI_CPP_
#define _HEMI_CPP_

#include "view.h"
#include "mesh.h"
#include "vectors.h"

#include "hemisphere.h"

//Default constructor
Hemisphere::Hemisphere() :
  levels(0),
  mesh(NULL),
  view(NULL)
{
  //Why would you use this?
}

//Regular constructor
Hemisphere::Hemisphere(Mesh* inmesh, int inlevels) :
  levels(inlevels),
  mesh(inmesh),
  view(NULL)
{
  //Much better
}

//Destructor
Hemisphere::~Hemisphere()
{
  //Only delete views if they exist
  if (view != NULL)
    {
      for (int i = 0; i < numViews(); i++)
	{
	  delete view[i];
	}
    }
}

//Initializes the data structure.
//Will not actually compute any views, but will get ready to do so.
//This must be called before the object can really be used.
void Hemisphere::setup()
{
  //First, compute the bounds of the mesh
  computeBounds();

  //Create the array of view pointers
  view = new View*[numViews()];
  std::cout << "Before view calculation\n";
  for (int i = 0; i < numViews(); i++)
    {
      //Create the View
      view[i] = new View(mesh);

      //Compute it
      Vec3f angles = getAngleFromIndex(i);
      view[i]->computeView(angles.x(), angles.y(), 100, min, max);
    }
  std::cout << "After view calculation\n";
}

//Computes the bounds of the mesh so each view doesn't have to
void Hemisphere::computeBounds()
{
  min = Vec3f(FLT_MAX, FLT_MAX, FLT_MAX);
  max = Vec3f(FLT_MIN, FLT_MIN, FLT_MIN);
  for (int i = 0; i < mesh->numVertices(); i++)
    {
      Vec3f pos = mesh->getVertex(i)->getPos();
      if (pos.x() > max.x()) max.setx(pos.x());
      if (pos.y() > max.y()) max.sety(pos.y());
      if (pos.z() > max.z()) max.setz(pos.z());
      if (pos.x() < min.x()) min.setx(pos.x());
      if (pos.y() < min.y()) min.sety(pos.y());
      if (pos.z() < min.z()) min.setz(pos.z());
    }
}

//Returns the number of views at the given level
int Hemisphere::viewsAtLevel(int l)
{
  //Invert l so the lowest number is the top
  l = levels - l;

  //Return the number
  if (l == 0) return 1;
  return std::pow(2, l+1);
}

//Returns the nearest view
View* Hemisphere::getNearestView(float angXZ, float angY)
{
  //Find the corresponding level for angY
  int ylevel = angY*((levels)/(HEMISPHERE_PI/2));

  //Find the corresponding point for angXZ
  int xzlevel = angXZ*(viewsAtLevel(ylevel)/(2*HEMISPHERE_PI));

  //Find the corresponding index
  int index = viewsAtLevel(ylevel-1) + xzlevel;
  if (index < 0 || index >= numViews())
    {
      std::cout << "Bad getNearestView\n";
      exit(0);
    }

  //Return the view at that point
  return view[index];
}

//Returns a Vec3f where x is the xz angle and y is the y angle
//Returns the angles that correspond to the index
Vec3f Hemisphere::getAngleFromIndex(int index)
{
  //Find the components of this index
  int ylevel = levels;
  int xzlevel = 0;
  while(index >= viewsAtLevel(ylevel))
    {
      index -= viewsAtLevel(ylevel);
      ylevel--;
    }
  xzlevel = index;

  //Find the corresponding angles
  float angXZ = (2.0*HEMISPHERE_PI*float(xzlevel)) / float(viewsAtLevel(ylevel));
  float angY = (0.5*HEMISPHERE_PI*float(ylevel)) / float(levels);

  //Construct and return a Vec3f
  return Vec3f(angXZ, angY, 0);
}

#endif
