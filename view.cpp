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
//This is a simpler set of parameters
void View::computeView(float angXZ, float angY, int distance)
{
  //Create the min and max vectors
  Vec3f min(FLT_MAX, FLT_MAX, FLT_MAX);
  Vec3f max(FLT_MIN, FLT_MIN, FLT_MIN);

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
  
  //Do the actual call
  computeView(angXZ, angY, distance, min, max);
}

//Computes a view of the mesh from the given angle and distance
void View::computeView(float angXZ, float angY, int distance, Vec3f min, Vec3f max)
{
  //From this, find the center point, base point, and camera direction
  Vec3f center((min.x()+max.x())/2, (min.y()+max.y())/2, (min.z()+max.z())/2);
  Vec3f base3d((min.x()+max.x())/2, min.y(), (min.z()+max.z())/2);
  Vec3f cameraDir(cos(angXZ)*(1-sin(angY)), sin(angY), sin(angXZ)*(1-sin(angY)));
  cameraDir.Normalize();

  //Find the position of the camera and the direction it looks
  Vec3f cameraPos(center + cameraDir*distance);
  cameraDir *= -1;

  //Find the size of the view as the largest distance in an axis
  float size = std::max(std::max(max.x()-min.x(), max.y()-min.y()), max.z()-min.z());
  size *= 1.1;

  //Make the camera
  OrthographicCamera camera(cameraPos, center, Vec3f(0,1,0), size);

  // Clear the display buffer, set it to the background color
  Vec3f bg = mesh->background_color;
  glClearColor(bg.r(),bg.g(),bg.b(),0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //Set the camera parameters
  camera.glInit(VIEW_SIZE, VIEW_SIZE);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  camera.glPlaceCamera();

  //Set the viewport for these renders
  int oldWidth = glutGet(GLUT_WINDOW_WIDTH);
  int oldHeight = glutGet(GLUT_WINDOW_HEIGHT);
  glViewport(0,0,VIEW_SIZE,VIEW_SIZE);

  //Set up OpenGL states
  glDisable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);

  HandleGLError("Before FBO");

  //Create the color FBO and depth RB
  GLuint color_FBO;
  GLuint depth_RB;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, VIEW_SIZE, VIEW_SIZE, 0, GL_RGB, GL_FLOAT, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
    
  glGenFramebuffers(1, &color_FBO);
  glBindFramebuffer(GL_FRAMEBUFFER, color_FBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			 GL_TEXTURE_2D, texture, 0);

  glGenRenderbuffers(1, &depth_RB);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_RB);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
			VIEW_SIZE, VIEW_SIZE);
  
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			    GL_RENDERBUFFER, depth_RB);

  //Ensure the FBO set up properly
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      std::cerr << "FBO setup failed\n";
      exit(0);
    }
    

  HandleGLError("After setting up FBO");
  
  //Render as usual
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDepthFunc(GL_LESS);
  mesh->drawVBOs();

  //Get the texture data
  float texdata[VIEW_SIZE*VIEW_SIZE*3];
  glBindTexture(GL_TEXTURE_2D, texture);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, texdata);

  //Get the depth data
  float depthdata[VIEW_SIZE*VIEW_SIZE];
  glReadPixels(0, 0, VIEW_SIZE, VIEW_SIZE, GL_DEPTH_COMPONENT, GL_FLOAT, depthdata);
  
  //Copy it over to the view data
  for (int i = 0; i < VIEW_SIZE*VIEW_SIZE; i++)
    {
      data[i].color = Vec3f(texdata[3*i], texdata[(3*i)+1], texdata[(3*i)+2]);
      data[i].mind = depthdata[i];
      if (data[i].mind == 1) {data[i].opacity = 0;}
      else {data[i].opacity = 1;}
    }

  //Render again, but with depth function set to GL_GREATER for maximum distance
  glDepthFunc(GL_GREATER);
  mesh->drawVBOs();
  glDepthFunc(GL_LESS);

  //Get the new depth data
  glReadPixels(0, 0, VIEW_SIZE, VIEW_SIZE, GL_DEPTH_COMPONENT, GL_FLOAT, depthdata);
  
  //Copy it over to the view data
  for (int i = 0; i < VIEW_SIZE*VIEW_SIZE; i++)
    {
      if (depthdata[i] != 0 && depthdata[i] != 1)
	{
	  std::cout << "woo\n";
	}
      data[i].maxd = depthdata[i];
    }

  //Unbind buffers
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  //Delete framebuffer and renderbuffer
  glDeleteFramebuffers(1, &color_FBO);
  glDeleteRenderbuffers(1, &depth_RB);

  //Reset viewport
  glViewport(0,0,oldWidth,oldHeight);

  HandleGLError("Leaving computeView");
}
