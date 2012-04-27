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
  mesh(NULL)
{
  //Why would you use this?
  view.resize(0);
}

//Regular constructor
Hemisphere::Hemisphere(Mesh* inmesh, int inlevels, int inpoints) :
  levels(inlevels),
  basepoints(inpoints),
  mesh(inmesh)
{
  //Much better
  view.resize(inlevels);
  for (unsigned int i = 0; i < view.size(); i++)
    {
      view[i].resize(0);
    }
}

//Destructor
Hemisphere::~Hemisphere()
{
  //Only delete views if they exist
  for (unsigned int i = 0; i < view.size(); i++)
    {
      for (unsigned int j = 0; j < view[i].size(); j++)
	{
	  if (view[i][j] != NULL)
	    {
	      delete view[i][j];
	    }
	}
    }
}

//Returns the number of views on the hemisphere
int Hemisphere::numViews()
{
  int sum = 0;
  for (unsigned int i = 0; i < view.size(); i++)
    {
      sum += view[i].size();
    }
  
  return sum;
}

//Initializes the data structure.
//Will not actually compute any views, but will get ready to do so.
//This must be called before the object can really be used.
void Hemisphere::setup()
{
  //First, compute the bounds of the mesh
  computeBounds();

  //Create the views
  //Cycle over each level
  std::cout << "Before view calculation\n";
  for (int i = 0; i < levels; i++)
    {
      //Find the number of points at this level
      float angY = (HEMISPHERE_PI/2)*(float(i)/float(levels-1));
      if (i == levels-1) angY -= 0.0001;
      view[i].resize(basepoints, NULL);

      //Compute the views
      for (unsigned int j = 0; j < view[i].size(); j++)
	{
	  float angXZ = (HEMISPHERE_PI*2)*(float(j)/float(view[i].size()));
	  view[i][j] = new View(mesh);
	  view[i][j]->computeView(angXZ, angY, 100, min, max);
	}
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

//Returns the nearest view given the angle to that view
View* Hemisphere::getNearestView(float angXZ, float angY)
{
  //Find the corresponding level for angY, rounding to nearest
  int ylevel = (angY*((levels-1)/(HEMISPHERE_PI/2))) + 0.5;

  //Just make sure it doesn't round too far
  if (ylevel == levels) ylevel--;

  //Find the corresponding point for angXZ, also rounding
  unsigned int xzlevel = (angXZ*(view[ylevel].size())/(2*HEMISPHERE_PI)) + 0.5;

  //Just make sure it doesn't round too far
  if (xzlevel == view[ylevel].size()) xzlevel--;

  //Return the view at that point
  return view[ylevel][xzlevel];
}

//Returns the nearest view given the position of the center
//and the position of the camera
View* Hemisphere::getNearestView(Vec3f pos, Vec3f camera)
{
  //Find a vector pointing to the camera from the center
  Vec3f toCamera = camera - pos;
  toCamera.Normalize();

  //Convert this to spherical coordinates
  Vec3f xzCamera(toCamera.x(), 0, toCamera.z());
  xzCamera.Normalize();
  float angXZ = std::acos(xzCamera.x());
  if (toCamera.z() < 0) angXZ *= -1;
  float angY = std::asin(toCamera.y());

  //Bound them to the hemisphere
  while (angXZ < 0) angXZ += 2*HEMISPHERE_PI;
  if (angY < 0) angY = 0;

  //Get the nearest view
  return getNearestView(angXZ, angY);
}

#endif
