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
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, color_FBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			 GL_TEXTURE_2D, texture, 0);

  glGenRenderbuffers(1, &depth_RB);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_RB);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16,
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
  mesh->drawVBOs(true);
  
  //Unbind buffers
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  //Get the texture data
  float texdata[VIEW_SIZE*VIEW_SIZE*3];
  glBindTexture(GL_TEXTURE_2D, texture);
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, texdata);
  
  //Copy it over to the view data
  for (int i = 0; i < VIEW_SIZE*VIEW_SIZE; i++)
    {
      data[i].color = Vec3f(texdata[3*i], texdata[(3*i)+1], texdata[(3*i)+2]);
    }

  //Reset viewport
  glViewport(0,0,oldWidth,oldHeight);

  HandleGLError("Leaving computeView");
}
