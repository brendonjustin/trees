#ifndef _GL_CANVAS_H_
#define _GL_CANVAS_H_

#include <cstdio>
#include <cstdlib>
#include <string>

// Included files for OpenGL Rendering
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#define GL_GLEXT_PROTOTYPES
#ifdef _WIN32
#define GLUT_DISABLE_ATEXIT_HACK
#include <GL/glew.h>
#endif
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#ifdef _WIN32
#include "GL/wglew.h"
#endif
#endif

#include "vectors.h"

class ArgParser;
class Mesh;
class Camera;
class Hemisphere;
class Forest;

// ====================================================================
// NOTE:  All the methods and variables of this class are static
// ====================================================================

class GLCanvas {

public:

  // Set up the canvas and enter the rendering loop
  // Note that this function will not return but can be
  // terminated by calling 'exit(0)'
  static void initialize(ArgParser *_args, Mesh* _mesh, Hemisphere* _hemisphere, Forest* _forest);
private:

  static void InitLight();

  // various static variables
  static ArgParser *args;
  static Mesh* mesh;
  static Camera *camera;
  static Hemisphere* hemisphere;
  static Forest* forest;

  // state of the mouse cursor
  static int mouseButton;
  static int mouseX;
  static int mouseY;
  static bool shiftPressed;
  static bool controlPressed;
  static bool altPressed;

  //State of some keys
  static bool key_w;
  static bool key_a;
  static bool key_s;
  static bool key_d;

  // Callback functions for mouse and keyboard events
  static void display(void);
  static void reshape(int w, int h);
  static void mouse(int button, int state, int x, int y);
  static void motion(int x, int y);
  static void keyboard(unsigned char key, int x, int y);
  static void keyboardUp(unsigned char key, int x, int y);
  static void idle();
};

// ====================================================================

int HandleGLError(const std::string &message = "");

#endif
