#include "glCanvas.h"

#include <iostream> 
#include "argparser.h"
#include "mesh.h"

// =========================================
// =========================================

int main(int argc, char *argv[]) {
  ArgParser args(argc, argv);
  Mesh mesh(&args);

  mesh.Load(args.input_file);
  glutInit(&argc,argv);
  GLCanvas::initialize(&args,&mesh); 

  return 0;
}

// =========================================
// =========================================
