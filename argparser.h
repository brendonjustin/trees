#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <string>
#include <cassert>
#include "MersenneTwister.h"

// ====================================================================================

class ArgParser {

public:

  ArgParser() { DefaultValues(); }
  
  ArgParser(int argc, char *argv[]) {
    DefaultValues();
    for (int i = 1; i < argc; i++) {
      if (argv[i] == std::string("-input")) {
        i++; assert (i < argc); 
        input_file = argv[i];
      } else if (argv[i] == std::string("-size")) {
        i++; assert (i < argc); 
        width = height = atoi(argv[i]);
      } else if (argv[i] == std::string("-gouraud")) {
        gouraud = true;
      } else if (argv[i] == std::string("-trees")) {
        i++; assert (i < argc); 
        trees = atoi(argv[i]);
      } else {
        printf ("whoops error with command line argument %d: '%s'\n",i,argv[i]);
        assert(0);
      }
    }
  }

  void DefaultValues() {
    width = 500;
    height = 500;
    gouraud = false;
    trees = 10;
  }

  // ==============
  // REPRESENTATION
  // all public! (no accessors)
  std::string input_file;
  int width;
  int height;
  bool gouraud;
  int trees;
  MTRand mtrand;

};

// ====================================================================================

#endif
