/*
  -----Tree View Class Implementation-----
  Auston Sterling
  Brendon Justin

  The implementation of the View class.
*/

#include "view.h"
#include <cfloat>

//TEST
#include <iostream>

//Default constructor
View::View() :
  mesh(NULL)
{
}

//Another constructor
View::View(Mesh* inmesh) :
  mesh(inmesh)
{
}

//Computes a view of the mesh from the given angle and distance
void View::computeView(float angXZ, float angY, int distance)
{
  //Create an orthographic camera to look at the model
  //Find the minimum and maximum values in each direction
  float xmax = FLT_MIN;
  float xmin = FLT_MAX;
  float ymax = FLT_MIN;
  float ymin = FLT_MAX;
  float zmax = FLT_MIN;
  float zmin = FLT_MAX;
  
  for (int i = 0; i < mesh->numVertices(); i++)
    {
      if (mesh->getVertex(i)->x() > xmax) xmax = mesh->getVertex(i)->x();
      if (mesh->getVertex(i)->y() > ymax) ymax = mesh->getVertex(i)->y();
      if (mesh->getVertex(i)->z() > zmax) zmax = mesh->getVertex(i)->z();
      if (mesh->getVertex(i)->x() < xmin) xmin = mesh->getVertex(i)->x();
      if (mesh->getVertex(i)->y() < ymin) ymin = mesh->getVertex(i)->y();
      if (mesh->getVertex(i)->z() < zmin) zmin = mesh->getVertex(i)->z();
    }

  //From this, find the center point, base point, and camera direction
  Vec3f center((xmin+xmax)/2, (ymin+ymax)/2, (zmin+zmax)/2);
  Vec3f base3d((xmin+xmax)/2, ymin, (zmin+zmax)/2);
  Vec3f cameraDir(cos(angXZ), sin(angY), sin(angXZ));
  cameraDir.Normalize();

  //Find the position of the camera and the direction it looks
  Vec3f cameraPos(center + cameraDir*distance);
  cameraDir *= -1;

  //Find the size of the view as the largest distance in an axis
  float size = std::max(std::max(xmax-xmin, ymax-ymin), zmax-zmin);
  size *= 1.1;

  //Make the camera
  OrthographicCamera camera(cameraPos, center, Vec3f(0,1,0), size);

  //For each texel
  /*
  for (int i = 0; i < VIEW_SIZE; i++)
    {
      std::cout << "ROW " << i << std::endl;
      
      for (int j = 0; j < VIEW_SIZE; j++)
	{
	  if (i == 70) exit(0);
	  //Generate a ray
	  Ray r = camera.generateRay(double(i)/double(VIEW_SIZE),
				     double(j)/double(VIEW_SIZE));
	  Hit closeHit;
	  Hit farHit;

	  //For each triangle in the mesh
	  for (triangleshashtype::iterator k = mesh->triangles.begin(); k != mesh->triangles.end(); k++)
	    {
	      //Check for an intersection
	      Hit h;
	      k->second->intersect(r, h);

	      //Check for closest hit or furthest hit
	      if (h.getT() < closeHit.getT()) closeHit = h;
	      if (h.getT() > farHit.getT() || farHit.getT() == FLT_MAX) farHit = h;
	    }
	  
	  data[i][j].mind = closeHit.getT();
	  data[i][j].maxd = farHit.getT();
	  if (closeHit.getMaterial() != NULL)
	    {
	      data[i][j].color = closeHit.getMaterial()->getDiffuseColor(closeHit.get_s(), closeHit.get_t());
	      data[i][j].opacity = 1.0;
	    }
	  else
	    {
	      data[i][j].color = Vec3f(0,0,0);
	      data[i][j].opacity = 0;
	    }
	}
    }
  */
}
