//
//  seeder.h
//  trees
//
//  Created by Brendon Justin on 4/19/12.
//  Copyright (c) 2012 Brendon Justin. All rights reserved.
//

#ifndef trees_seeder_h
#define trees_seeder_h

#include "vectors.h"

#include <vector>

class Seeder {
  double m_lambda;
  static const int factorial[];
  std::vector<int> getDistribution(float area, int numBlocks);
  
public:
  Seeder(double expectedNum) : m_lambda(expectedNum) {};
  std::vector<Vec3f> getTreeLocations(float area, int numBlocks);
  
};

#endif
