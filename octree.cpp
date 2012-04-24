/*
  -----Octree Class Implementation-----
  Auston Sterling
  Brendon Justin

  An octree for storing triangles for quick access during raytracing.
*/

#include "octree.h"
#include "vertex.h"

#include <cfloat>

//Default constructor
Octree::Octree()
{
  //Default values work fine in most cases.
  //Probably shouldn't use this constructor anyway
  for (int i = 0; i < 8; i++)
    {
      child[i] = NULL;
    }
  
  maxTris = 0;
}

//Standard constructor
Octree::Octree(Vec3f inmin, Vec3f inmax, unsigned short inmaxTris) :
  min(inmin),
  max(inmax),
  maxTris(inmaxTris)
{
  for (int i = 0; i < 8; i++)
    {
      child[i] = NULL;
    }
}

//Destructor
Octree::~Octree()
{
  //Delete all children recursively
  for (int i = 0; i < 8; i++)
    {
      if (child[i] != NULL) delete child[i];
    }
}

//Adds a triangle to the box after ensuring it intersects
//Returns true if it added successfully
bool Octree::addTriangle(Triangle* tri)
{
  //If this is a leaf, add to child nodes instead
  if (child[0] != NULL)
    {
      bool ret = false;
      for (int i = 0; i < 8; i++)
	{
	  if (child[i]->addTriangle(tri) == true)
	    {
	      ret = true;
	    }
	}
      
      return ret;
    }
  
  //For speed, only add a triangle if one of its points is in the box
  //This could have some misses, but it's faster and easier
  Vec3f a = tri->getEdge()->getStartVertex()->getPos();
  Vec3f b = tri->getEdge()->getNext()->getStartVertex()->getPos();
  Vec3f c = tri->getEdge()->getNext()->getNext()->getStartVertex()->getPos();

  bool intersect = false;
  if (a.x() < max.x() && a.x() > min.x() &&
      a.y() < max.y() && a.y() > min.y() &&
      a.z() < max.z() && a.z() > min.z())
    {
      intersect = true;
    }

  if (b.x() < max.x() && b.x() > min.x() &&
      b.y() < max.y() && b.y() > min.y() &&
      b.z() < max.z() && b.z() > min.z())
    {
      intersect = true;
    }

  if (c.x() < max.x() && c.x() > min.x() &&
      c.y() < max.y() && c.y() > min.y() &&
      c.z() < max.z() && c.z() > min.z())
    {
      intersect = true;
    }

  //If there's no intersection, don't add it
  if (!intersect) return false;

  //Add the triangle to the vector
  triangles.insert(tri);

  //Check for overflow
  if (triangles.size() > maxTris)
    {
      //We need to split this node
      split();
    }

  //Successfully added
  return true;
}

//Splits a node up into the eight children
//This node acts as a parent, but will hold no triangles itself
//Split redistributes the triangles into the children
void Octree::split()
{
  //Ensure split isn't being called on an already-split node
  if (child[0] != NULL)
    {
      return;
    }

  //For each child
  for (int i = 0; i < 8; i++)
    {
      //Find the coordinates of the child
      Vec3f newmin = min;
      Vec3f newmax = (min + max) * 0.5;
      if (i%2 == 1)
	{
	  //Shift the box along x
	  newmin.setx(newmin.x() + (max.x() - min.x())/2);
	  newmax.setx(newmax.x() + (max.x() - min.x())/2);
	}
      if (i%4 > 1)
	{
	  //Shift along y
	  newmin.sety(newmin.y() + (max.y() - min.y())/2);
	  newmax.sety(newmax.y() + (max.y() - min.y())/2);
	}
      if (i > 3)
	{
	  //Shift along z
	  newmin.setz(newmin.z() + (max.z() - min.z())/2);
	  newmax.setz(newmin.z() + (max.z() - min.z())/2);
	}
      
      //Create the child
      child[i] = new Octree(newmin, newmax, maxTris);

      //Check to see which triangles should be in here
      for (std::set<Triangle*>::iterator j = triangles.begin(); j != triangles.end(); j++)
	{
	  child[i]->addTriangle((*j));
	}
    }

  //This node is now an intermediate node, and contains no triangles
  triangles.clear();
}

//Gets all the triangles at this node and below, returning a vector of them all
std::set<Triangle*> Octree::getTris()
{
  //Create the return vector
  std::set<Triangle*> ret;

  //If this is an intermediate node, recurse
  if (child[0] != NULL)
    {
      for (int i = 0; i < 8; i++)
	{
	  //Add the child's triangles to the return vector
	  std::set<Triangle*> tris = child[i]->getTris();
	  ret.insert(tris.begin(), tris.end());
	}
    }
  else //If it's a leaf node, just return the triangles
    {
      return triangles;
    }

  //Return the combination of all children
  return ret;
}

//Gets all the triangles at this node and below, returning a vector of them all
//This version only returns triangles which are in nodes which the ray intersects
//This code assumes the ray starts way outside the box
//There may be some Bad Stuff otherwise
std::set<Triangle*> Octree::getTris(const Ray & r)
{
  //Check if the ray intersects with this box
  //Find the time of intersection with the six planes
  //max.x() = r.getOrigin().x() + t * r.getDirection().x()
  //t = (max.x() - r.getOrigin())/r.getDirection().x()
  float xplus = (max.x() - r.getOrigin().x())/r.getDirection().x();
  float xmin = (min.x() - r.getOrigin().x())/r.getDirection().x();
  float yplus = (max.y() - r.getOrigin().y())/r.getDirection().y();
  float ymin = (min.y() - r.getOrigin().y())/r.getDirection().y();
  float zplus = (max.z() - r.getOrigin().z())/r.getDirection().z();
  float zmin = (min.z() - r.getOrigin().z())/r.getDirection().z();

  //Find the latest entry and earliest exit
  float entry = FLT_MIN;
  float exit = FLT_MAX;

  //Check for ray parallel to planes
  char skip = 0;
  if (r.getDirection().Dot3(Vec3f(0,1,1)) < 0.0001 &&
      r.getDirection().Dot3(Vec3f(0,1,1)) > -0.0001)
    {
      if (r.getOrigin().y() > min.y() && r.getOrigin().y() < max.y() &&
	  r.getOrigin().z() > min.z() && r.getOrigin().z() < max.z())
	{ skip = 1; }
      else { skip = 2; }
    }
  
  if (r.getDirection().Dot3(Vec3f(1,0,1)) < 0.0001 &&
      r.getDirection().Dot3(Vec3f(1,0,1)) > -0.0001)
    {
      if (r.getOrigin().x() > min.x() && r.getOrigin().x() < max.x() &&
	  r.getOrigin().z() > min.z() && r.getOrigin().z() < max.z())
	{ skip = 1; }
      else { skip = 2; }
    }
  if (r.getDirection().Dot3(Vec3f(1,1,0)) < 0.0001 &&
      r.getDirection().Dot3(Vec3f(1,1,0)) > -0.0001)
    {
      if (r.getOrigin().x() > min.x() && r.getOrigin().x() < max.x() &&
	  r.getOrigin().y() > min.y() && r.getOrigin().y() < max.y())
	{ skip = 1; }
      else { skip = 2; }
    }

  //Skip = 1: hit, skip = 2: miss
  if (skip == 2)
    {
      //This does not intersect
      std::set<Triangle*> ret;
      ret.clear();
      return ret;
    }

  if (skip == 0)
    {
      if (r.getDirection().x() > 0.0001 || r.getDirection().x() < -0.0001)
	{
	  if (r.getOrigin().x() > max.x())
	    {
	      if (xplus > entry) entry = xplus;
	    }
	  else
	    {
	      if (xplus < exit) exit = xplus;
	    }
	}

      if (r.getDirection().y() > 0.0001 || r.getDirection().y() < -0.0001)
	{
	  if (r.getOrigin().y() > max.y())
	    {
	      if (yplus > entry) entry = yplus;
	    }
	  else
	    {
	      if (yplus < exit) exit = yplus;
	    }
	}

      if (r.getDirection().z() > 0.0001 || r.getDirection().z() < -0.0001)
	{
	  if (r.getOrigin().z() > max.z())
	    {
	      if (zplus > entry) entry = zplus;
	    }
	  else
	    {
	      if (zplus < exit) exit = zplus;
	    }
	}

      if (r.getDirection().x() > 0.0001 || r.getDirection().x() < -0.0001)
	{
	  if (r.getOrigin().x() < min.x())
	    {
	      if (xmin > entry) entry = xmin;
	    }
	  else
	    {
	      if (xmin < exit) exit = xmin;
	    }
	}

      if (r.getDirection().y() > 0.0001 || r.getDirection().y() < -0.0001)
	{
	  if (r.getOrigin().y() < min.y())
	    {
	      if (ymin > entry) entry = ymin;
	    }
	  else
	    {
	      if (ymin < exit) exit = ymin;
	    }
	}

      if (r.getDirection().z() > 0.0001 || r.getDirection().z() < -0.0001)
	{
	  if (r.getOrigin().z() < min.z())
	    {
	      if (zmin > entry) entry = zmin;
	    }
	  else
	    {
	      if (zmin < exit) exit = zmin;
	    }
	}

      //If the entry value is larger than the exit value
      if (entry > exit)
	{
	  //This does not intersect
	  std::set<Triangle*> ret;
	  ret.clear();
	  return ret;
	}
    }

  //The ray intersects the triangle, proceed as usual
  
  //Create the return vector
  std::set<Triangle*> ret;

  //If this is an intermediate node, recurse
  if (child[0] != NULL)
    {
      for (int i = 0; i < 8; i++)
	{
	  //Add the child's triangles to the return vector
	  std::set<Triangle*> tris = child[i]->getTris(r);
	  ret.insert(tris.begin(), tris.end());
	}
    }
  else //If it's a leaf node, just return the triangles
    {
      return triangles;
    }

  //Return the combination of all children
  return ret;
}
