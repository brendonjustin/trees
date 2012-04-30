//
//  terraingenerator.h
//  trees
//
//  Created by Brendon Justin on 4/29/12.
//  Copyright (c) 2012 Brendon Justin. All rights reserved.
//

#ifndef trees_terraingenerator_h
#define trees_terraingenerator_h

#include <vector>

//	Based on pseudocode from http://gameprogrammer.com/fractal.html
class TerrainGenerator {
  static float ratio;
  static float getRandomOffset(int);
  static void diamondIteration(std::vector<float>&, int);
  static void squareIteration(std::vector<float>&, int);

public:
  static void setRatio(float newRatio) { ratio = newRatio; };
  static std::vector<float> generate(int squaresPerSide);
  
};

#endif