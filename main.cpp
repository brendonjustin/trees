#include "glCanvas.h"

#include <time.h>

#include "MersenneTwister.h"
#include <iostream> 
#include "argparser.h"
#include "mesh.h"
#include "hemisphere.h"
#include "forest.h"

MTRand GLOBAL_mtrand;

// =========================================
// =========================================

int main(int argc, char *argv[]) {

  // deterministic (repeatable) randomness
  GLOBAL_mtrand = MTRand(37);
  // "real" randomness
  // GLOBAL_mtrand = MTRand((unsigned)time(0));
  //
  
  ArgParser args(argc, argv);
  Mesh mesh(&args);
  Hemisphere hemisphere(&mesh, 10, 30);
  Forest forest(&args, &hemisphere);

  mesh.Load(args.input_file);
  glutInit(&argc,argv);
  GLCanvas::initialize(&args,&mesh,&hemisphere,&forest); 

  return 0;
}

// =========================================
// =========================================
