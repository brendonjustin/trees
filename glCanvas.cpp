#include "glCanvas.h"
#include "argparser.h"
#include "camera.h"
#include "mesh.h"
#include "hemisphere.h"
#include "forest.h"

#include "view.h"

#include <cmath>

// ========================================================
// static variables of GLCanvas class

ArgParser* GLCanvas::args = NULL;
Mesh* GLCanvas::mesh = NULL;
Camera* GLCanvas::camera = NULL;
Hemisphere* GLCanvas::hemisphere = NULL;
Forest* GLCanvas::forest = NULL;

// State of the mouse cursor
int GLCanvas::mouseButton = 0;
int GLCanvas::mouseX = 0;
int GLCanvas::mouseY = 0;
bool GLCanvas::controlPressed = false;
bool GLCanvas::shiftPressed = false;
bool GLCanvas::altPressed = false;

//State of some keys
bool GLCanvas::key_w = false;
bool GLCanvas::key_a = false;
bool GLCanvas::key_s = false;
bool GLCanvas::key_d = false;

// ========================================================
// Initialize all appropriate OpenGL variables, set
// callback functions, and start the main event loop.
// This function will not return but can be terminated
// by calling 'exit(0)'
// ========================================================

void GLCanvas::initialize(ArgParser *_args, Mesh* _mesh, Hemisphere* _hemisphere, Forest* _forest) {

  args = _args;
  mesh = _mesh;
  hemisphere = _hemisphere;
  forest = _forest;

  // Vec3f camera_position = Vec3f(0,0,5);
  Vec3f camera_position = Vec3f(50,10,-200);
  Vec3f point_of_interest = Vec3f(50,10,-195);
  // Vec3f point_of_interest = Vec3f(0,0,0);
  Vec3f up = Vec3f(0,1,0);
  camera = new PerspectiveCamera(camera_position, point_of_interest, up, 20 * M_PI/180.0);

  // setup glut stuff
  glutInitWindowSize(args->width, args->height);
  glutInitWindowPosition(100,100);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
  glutCreateWindow("OpenGL Viewer");
  HandleGLError("in glcanvas initialize");

#ifdef _WIN32
  GLenum err = glewInit();
  if (err != GLEW_OK) {
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
      exit(1);
  }
#endif
  // basic rendering 
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
  glShadeModel(GL_SMOOTH);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  GLfloat ambient[] = { 0.7, 0.7, 0.7, 1.0 };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
  glCullFace(GL_BACK);
  glDisable(GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);

  // Initialize callback functions
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutKeyboardUpFunc(keyboardUp);
  glutIdleFunc(idle);

  HandleGLError("finished glcanvas initialize");

  mesh->initializeVBOs();
  hemisphere->setup();

  forest->setCameraPosition(camera->getPosition());
  forest->initializeVBOs();

  HandleGLError("finished mesh, hemisphere, and forest initialization");

  // Enter the main rendering loop
  glutMainLoop();
}


// ========================================================

void GLCanvas::InitLight() {
  // Set the last component of the position to 0 to indicate
  // a directional light source

  GLfloat position[4] = { 30,30,100, 1};
  GLfloat diffuse[4] = { 0.75,0.75,0.75,1};
  GLfloat specular[4] = { 0,0,0,1};
  // GLfloat ambient[4] = { 0.2, 0.2, 0.2, 1.0 };
  GLfloat ambient[] = { 0.7, 0.7, 0.7, 1.0 };

  GLfloat zero[4] = {0,0,0,0};
  glLightfv(GL_LIGHT1, GL_POSITION, position);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
  glLightfv(GL_LIGHT1, GL_AMBIENT, zero);
  glEnable(GL_LIGHT1);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glEnable(GL_COLOR_MATERIAL);
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  GLfloat spec_mat[4] = {1,1,1,1};
  float glexponent = 30;
  glMaterialfv(GL_FRONT, GL_SHININESS, &glexponent);
  glMaterialfv(GL_FRONT, GL_SPECULAR, spec_mat);

  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  float back_color[] = { 0.2,0.8,0.8,1};
  glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, back_color);
  glEnable(GL_LIGHT1);
}


void GLCanvas::display(void) {
  glDrawBuffer(GL_BACK);

  mesh->background_color = Vec3f(1.0, 1.0, 1.0);
  Vec3f bg = mesh->background_color;
  // Clear the display buffer, set it to the background color
  glClearColor(bg.r(),bg.g(),bg.b(),0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Set the camera parameters
  camera->glInit(args->width, args->height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  camera->glPlaceCamera();
  InitLight(); // light will be a headlamp!
  
  //  draw the trees and the ground
  forest->drawVBOs();

  glGetError();
  HandleGLError(); 
   
  // Swap the back buffer with the front buffer to display
  // the scene
  glutSwapBuffers();
}

// ========================================================
// Callback function for window resize
// ========================================================

void GLCanvas::reshape(int w, int h) {
  args->width = w;
  args->height = h;

  // Set the OpenGL viewport to fill the entire window
  glViewport(0, 0, (GLsizei)args->width, (GLsizei)args->height);

  // Set the camera parameters to reflect the changes
  camera->glInit(args->width, args->height);
}

// ========================================================
// Callback function for mouse click or release
// ========================================================

void GLCanvas::mouse(int button, int /*state*/, int x, int y) {
  // Save the current state of the mouse.  This will be
  // used by the 'motion' function
  mouseButton = button;
  mouseX = x;
  mouseY = y;

  shiftPressed = (glutGetModifiers() & GLUT_ACTIVE_SHIFT) != 0;
  controlPressed = (glutGetModifiers() & GLUT_ACTIVE_CTRL) != 0;
  altPressed = (glutGetModifiers() & GLUT_ACTIVE_ALT) != 0;
}

// ========================================================
// Callback function for mouse drag
// ========================================================

void GLCanvas::motion(int x, int y) {
  // Left button = rotation
  // (rotate camera around the up and horizontal vectors)
  if (mouseButton == GLUT_LEFT_BUTTON) {
    camera->rotateCamera(0.005*(mouseX-x), 0.005*(mouseY-y));
    mouseX = x;
    mouseY = y;
  }
  // Middle button = translation
  // (move camera perpendicular to the direction vector)
  else if (mouseButton == GLUT_MIDDLE_BUTTON) {
    camera->truckCamera((mouseX-x)*0.5, (y-mouseY)*0.5);
    mouseX = x;
    mouseY = y;
    forest->cameraMoved(camera->getPosition());
  }
  // Right button = dolly or zoom
  // (move camera along the direction vector)
  else if (mouseButton == GLUT_RIGHT_BUTTON) {
    if (controlPressed) {
      camera->zoomCamera(mouseY-y);
    } else {
      camera->dollyCamera(mouseY-y);
    }
    mouseX = x;
    mouseY = y;
    forest->cameraMoved(camera->getPosition());
  }


  // Redraw the scene with the new camera parameters
  glutPostRedisplay();
}

// ========================================================
// Callback functions for keyboard events
// ========================================================

void GLCanvas::keyboard(unsigned char key, int x, int y) {
  switch (key) {
  case 'q': case 'Q':
    exit(0);
    break;
  case 'w': case 'W':
    key_w = true;
    break;
  case 's': case 'S':
    key_s = true;
    break;
  case 'a': case 'A':
    key_a = true;
    break;
  case 'd': case 'D':
    key_d = true;
    break;
  default:
    printf("UNKNOWN KEYBOARD INPUT  '%c'\n", key);
  }
}

void GLCanvas::keyboardUp(unsigned char key, int x, int y) {
  switch (key) {
  case 'q': case 'Q':
    exit(0);
    break;
  case 'w': case 'W':
    key_w = false;
    break;
  case 's': case 'S':
    key_s = false;
    break;
  case 'a': case 'A':
    key_a = false;
    break;
  case 'd': case 'D':
    key_d = false;
    break;
  default:
    printf("UNKNOWN KEYBOARD INPUT  '%c'\n", key);
  }
}

void GLCanvas::idle() {
  //This is where radiosity->iterate or DrawPixels() is called
  if (key_w && !key_s)
    {
      camera->dollyCameraAndPoI(1);
    }
  if (!key_w && key_s)
    {
      camera->dollyCameraAndPoI(-1);
    }
  if (key_a && !key_d)
    {
      camera->truckCamera(-50, 0);
    }
  if (!key_a && key_d)
    {
      camera->truckCamera(50, 0);
    }
  if (key_w || key_a || key_s || key_d)
    {
      forest->cameraMoved(camera->getPosition());
      glutPostRedisplay();
    }
  
  glEnd();
  glFlush();
}

// ========================================================
// ========================================================

int HandleGLError(const std::string &message) {
  GLenum error;
  int i = 0;
  while ((error = glGetError()) != GL_NO_ERROR) {
    if (message != "") {
      std::cout << "[" << message << "] ";
    }
    std::cout << "GL ERROR(" << i << ") " << gluErrorString(error) << std::endl;
    i++;
  }
  if (i == 0) return 1;
  return 0;
}

// ========================================================
// ========================================================
