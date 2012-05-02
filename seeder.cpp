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

std::vector<std::vector<Vec3f> > Seeder::getTreeLocations(float area, int numBlocks)
{
  //  The scene's ground
  float blockSideLength;
  int sqrtNumBlocks, numTrees;
  Seeder seeder = Seeder(2);
  std::vector<int> pointsPerBlock;
  Vec3f cellOffset, intraCellOffset;
  
  blockSideLength = sqrt(area / numBlocks);
  sqrtNumBlocks = (int)sqrt(numBlocks);
  std::vector<std::vector<Vec3f> > locations (numBlocks);
  
  pointsPerBlock = this->getPoissonDistribution(numBlocks);
  
  //  Distribute trees at n per block
  for (int i = 0; i < numBlocks; ++i) {
    cellOffset = Vec3f((i % sqrtNumBlocks) * blockSideLength, 0, 2 * (i / sqrtNumBlocks) * blockSideLength);
    numTrees = pointsPerBlock[i];
    for (int j = 0; j < numTrees; ++j) {
      intraCellOffset = Vec3f(blockSideLength / 5, 0, blockSideLength / 5) + j*Vec3f(blockSideLength / numTrees,0,0) + (j % (int)m_lambda)*Vec3f(0,0,(int)m_lambda * blockSideLength / numTrees);
      locations[i].push_back(cellOffset + intraCellOffset);
    }
  }
  
  return locations;
}
