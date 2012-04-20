#include "glCanvas.h"

#include <time.h>

#include "MersenneTwister.h"
#include <iostream> 
#include "argparser.h"
#include "mesh.h"

MTRand GLOBAL_mtrand;

// =========================================
// =========================================

int main(int argc, char *argv[]) {

  // deterministic (repeatable) randomness
  GLOBAL_mtrand = MTRand(37);
  // "real" randomness
  //GLOBAL_mtrand = MTRand((unsigned)time(0));
  //
  
  ArgParser args(argc, argv);
  Mesh mesh(&args);

  mesh.Load(args.input_file);
  glutInit(&argc,argv);
  GLCanvas::initialize(&args,&mesh); 

  return 0;
}

// =========================================
// =========================================
