//
//  seeder.cpp
//  trees
//
//  Created by Brendon Justin on 4/21/12.
//  Copyright (c) 2012 Brendon Justin. All rights reserved.
//

#include "seeder.h"
#include "utils.h"

#include "mesh.h"

#include <cmath>
#include <iostream>

//  precomputed factorial values for 0-10
const int Seeder::factorial[] = {
  1,
  1,
  2,
  6,
  24,
  120,
  720,
  5040,
  40320,
  362880,
  3628800,
};

std::vector<int> Seeder::getPoissonDistribution(int numBlocks)
{
  double rand;
  double sum = 0;
  int maxK = 10;
  int i = 0;
  std::vector<int> pointsPerBlock;
  
  for (int a = 0; a < numBlocks; ++a) {
    rand = GLOBAL_mtrand.rand();
    sum = 0;
    for (i = 0; i < maxK; ++i) {
      sum += pow(m_lambda, i)*exp(-m_lambda) / factorial[i];
      
      if (sum > rand) {
        break;
      }
    }
    pointsPerBlock.push_back(i);
  }
  
  return pointsPerBlock;
}

std::vector<std::vector<Vec3f> > Seeder::getTreeLocations(float area, int numBlocks, float treeSize)
{
  //  The scene's ground
  float blockSideLength, randOffset1, randOffset2;
  int numTrees;
  std::vector<int> pointsPerBlock;
  Vec3f intraCellOffset;
  
  blockSideLength = sqrt(area / numBlocks);
  std::vector<std::vector<Vec3f> > locations (numBlocks);
  
  pointsPerBlock = this->getPoissonDistribution(numBlocks);
  
  //  Distribute trees at n per block
  for (int i = 0; i < numBlocks; ++i) {
    numTrees = pointsPerBlock[i];
    for (int j = 0; j < numTrees; ++j) {
      randOffset1 = (GLOBAL_mtrand.rand() + 0.5);
      randOffset2 = (GLOBAL_mtrand.rand() + 0.5);
      intraCellOffset = Vec3f(randOffset1 * blockSideLength / numTrees, 0, randOffset2 * blockSideLength / numTrees) 
                        + (j / (int)m_lambda)*Vec3f(randOffset1 * blockSideLength / numTrees,0,randOffset2 * blockSideLength / (numTrees*2))
                        + (j % (int)m_lambda)*Vec3f(randOffset2 * blockSideLength / (numTrees*2),0,randOffset1 * blockSideLength / numTrees);
      locations[i].push_back(intraCellOffset);
    }
  }
  
  return locations;
}
