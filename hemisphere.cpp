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

//Returns the interpolated view given the position of the center
//and the position of the camera
View* Hemisphere::getInterpolatedView(Vec3f pos, Vec3f camera)
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

  //Get the interpolated view
  return getInterpolatedView(angXZ, angY);
}

//Returns an interpolated view of the tree given the angle between the
//model and the camera
View* Hemisphere::getInterpolatedView(float angXZ, float angY)
{
  //Get the level coordinates from the angles
  float ycoord = angY*((float(levels)-1.0)/(HEMISPHERE_PI/2.0));
  float xzcoord = angXZ*(float(view[int(ycoord)].size()))/(2.0*HEMISPHERE_PI);
  
  //Find the corresponding level for angY, rounding down
  int ylevel = int(ycoord);

  //Find the corresponding point for angXZ, also rounding down
  unsigned int xzlevel = (unsigned int)(xzcoord);

  //Find the four surrounding points
  int ylevel2 = ylevel+1;
  if (ylevel2 == levels) ylevel2 = 0;
  int xzlevel2 = xzlevel+1;
  if (xzlevel2 == int(view[ylevel].size())) xzlevel2 = 0;

  //Discard the one that's furthest away to get the three nearest views
  int xz[3];
  int y[3];
  float w[3];
  
  if ((angY*((levels-1)/(HEMISPHERE_PI/2))) - ylevel < 0.5)
    {
      y[0] = ylevel;
      y[1] = ylevel2;
      y[2] = ylevel;
      if ((angXZ*(view[ylevel].size())/(2*HEMISPHERE_PI)) - xzlevel < 0.5)
	{
	  xz[0] = xzlevel;
	  xz[1] = xzlevel;
	  xz[2] = xzlevel2;
	}
      else
	{
	  xz[0] = xzlevel2;
	  xz[1] = xzlevel2;
	  xz[2] = xzlevel;
	}
    }
  else
    {
      y[0] = ylevel2;
      y[1] = ylevel;
      y[2] = ylevel2;
      if ((angXZ*(view[ylevel].size())/(2*HEMISPHERE_PI)) - xzlevel < 0.5)
	{
	  xz[0] = xzlevel;
	  xz[1] = xzlevel;
	  xz[2] = xzlevel2;
	}
      else
	{
	  xz[0] = xzlevel2;
	  xz[1] = xzlevel2;
	  xz[2] = xzlevel;
	}
    }

  //Compute the weights
  float sum = std::sqrt((ycoord-y[0])*(ycoord-y[0]) + (xzcoord-xz[0])*(xzcoord-xz[0])) +
    std::sqrt((ycoord-y[1])*(ycoord-y[1]) + (xzcoord-xz[1])*(xzcoord-xz[1])) +
    std::sqrt((ycoord-y[2])*(ycoord-y[2]) + (xzcoord-xz[2])*(xzcoord-xz[2]));
  w[0] = std::sqrt((ycoord-y[0])*(ycoord-y[0]) + (xzcoord-xz[0])*(xzcoord-xz[0]))/sum;
  w[1] = std::sqrt((ycoord-y[1])*(ycoord-y[1]) + (xzcoord-xz[1])*(xzcoord-xz[1]))/sum;
  w[2] = std::sqrt((ycoord-y[2])*(ycoord-y[2]) + (xzcoord-xz[2])*(xzcoord-xz[2]))/sum;

  //Get the views
  View* v[3];
  v[0] = view[y[0]][xz[0]];
  v[1] = view[y[1]][xz[1]];
  v[2] = view[y[2]][xz[2]];

  //Find the tree center
  //The normal vector to the plane perpendicular to the interpolated view is
  //a vector in the direction of angY and angXZ
  Vec3f center = (min+max)/2;

  //Create a camera for ray casting
  Vec3f cameraDir(cos(angXZ)*(1-sin(angY)), sin(angY), sin(angXZ)*(1-sin(angXZ)));
  cameraDir.Normalize();
  int distance = 100;
  Vec3f cameraPos(center + cameraDir*distance);

  //Find the size of the view as the largest distance in an axis
  float size = std::max(std::max(max.x()-min.x(), max.y()-min.y()), max.z()-min.z());	
  OrthographicCamera camera(cameraPos, center, Vec3f(0,1,0), size);

  //For each pixel
  for (int i = 0; i < VIEW_SIZE; i++)
    {
      for (int j = 0; j < VIEW_SIZE; j++)
	{
	  //Cast a ray
	  Ray r = camera.generateRay(double(i)/double(VIEW_SIZE),
				     double(j)/double(VIEW_SIZE));

	  //Find p, the intersection of the view ray with the interpolated plane
	  Vec3f p = r.pointAtParameter(distance);

	  //Project p onto the three nearest views and interpolate to find q
	  texel proj[3];

	  proj[0] = getNearestTexel(projectPoint(p, center,
						 getXZAngFromLevel(xz[0]),
						 getYAngFromLevel(y[0])),
				    center, getXZAngFromLevel(xz[0]),
				    getYAngFromLevel(y[0]));
	  proj[1] = getNearestTexel(projectPoint(p, center,
						 getXZAngFromLevel(xz[0]),
						 getYAngFromLevel(y[0])),
				    center, getXZAngFromLevel(xz[0]),
				    getYAngFromLevel(y[0]));
	  proj[2] = getNearestTexel(projectPoint(p, center,
						 getXZAngFromLevel(xz[0]),
						 getYAngFromLevel(y[0])),
				    center, getXZAngFromLevel(xz[0]),
				    getYAngFromLevel(y[0]));
	  Vec3f q = p + r.getDirection()*((w[0]*proj[0].mind+w[1]*proj[1].mind+w[2]*proj[2].mind) / (w[0]*proj[0].opacity+w[1]*proj[1].opacity+w[2]*proj[2].opacity));

	  //Now project q to find s
	  proj[0] = getNearestTexel(projectPoint(q, center,
						 getXZAngFromLevel(xz[0]),
						 getYAngFromLevel(y[0])),
				    center, getXZAngFromLevel(xz[0]),
				    getYAngFromLevel(y[0]));
	  proj[1] = getNearestTexel(projectPoint(q, center,
						 getXZAngFromLevel(xz[0]),
						 getYAngFromLevel(y[0])),
				    center, getXZAngFromLevel(xz[0]),
				    getYAngFromLevel(y[0]));
	  proj[2] = getNearestTexel(projectPoint(q, center,
						 getXZAngFromLevel(xz[0]),
						 getYAngFromLevel(y[0])),
				    center, getXZAngFromLevel(xz[0]),
				    getYAngFromLevel(y[0]));
	  Vec3f s = q + r.getDirection()*((w[0]*proj[0].mind+w[1]*proj[1].mind+w[2]*proj[2].mind) / (w[0]*proj[0].opacity+w[1]*proj[1].opacity+w[2]*proj[2].opacity));
	  Vec3f outcolor = w[0]*proj[0].color+w[1]*proj[1].color+w[2]*proj[2].color;
	}
    }
}

//Projects the input point p to a plane containing point center and
//with normal vector described by the input angles
Vec3f Hemisphere::projectPoint(Vec3f p, Vec3f center, float angXZ, float angY)
{
  //Create a vector from center to point and the normal vector
  Vec3f toP(p - center);
  Vec3f norm(std::sin(angXZ)*(1-std::sin(angY)), std::sin(angY),
	     std::cos(angXZ)*(1-std::sin(angY)));

  //Project toP onto the normal, subtract it from p, and return it
  return p - ((toP.Dot3(norm)/norm.Dot3(norm))*norm);
}

//Returns the texel nearest to the point p in the view at xzlevel, ylevel
texel Hemisphere::getNearestTexel(Vec3f p, Vec3f center, float angXZ, float angY)
{
  //Get the view at that level
  View* v = getNearestView(angXZ, angY);

  //Create a vector toward the camera
  Vec3f norm(std::sin(angXZ)*(1-std::sin(angY)), std::sin(angY),
	     std::cos(angXZ)*(1-std::sin(angY)));

  //Find the size
  float size = std::max(std::max(max.x()-min.x(), max.y()-min.y()), max.z()-min.z());

  //Find one of the vectors orthogonal to the normal (Right)
  Vec3f right;
  Vec3f::Cross3(right, norm, Vec3f(0,1,0));
  right.Normalize();

  //Find the other orthogonal vector (Up)
  Vec3f up;
  Vec3f::Cross3(up, right, norm);
  up.Normalize();

  //Shift the origin
  p += (size/2)*up;
  p += (size/2)*right;
  p = p - center;

  //p = center + A*Up + B*Right
  //p-center = A*Up + B*Right
  Vec3f proj = (p.Dot3(up)/up.Dot3(up))*up;
  int y = proj.Length();
  p -= proj;
  int x = p.Length();
  if (y > VIEW_SIZE) y = VIEW_SIZE;
  if (x > VIEW_SIZE) x = VIEW_SIZE;
  return v->getTexel(y, x);
}
#endif
