//
//  seeder.cpp
//  trees
//
//  Created by Brendon Justin on 4/21/12.
//  Copyright (c) 2012 Naga Softworks, LLC. All rights reserved.
//

#include "seeder.h"
#include "utils.h"

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

void Seeder::getDistribution(float area, float blockSize, std::vector<int>& numPerBlock)
{
  MTRand mtrand = MTRand();
  double rand = GLOBAL_mtrand.rand();
  double sum = 0;
  int maxK = 10;
  int i = 0;
  
  for (int a = 0; a < area / blockSize; ++a) {
    for (i = 0; i < maxK; ++i) {
      //  two-dimensional poisson distribution sibstitutes lambda*area
      //  everywhere lambda appears
      sum += exp(-m_lambda*blockSize)*pow(m_lambda*blockSize, i) / factorial[i];
      
      if (sum > rand) {
        numPerBlock.push_back(i);
        break;
      }
    }
  }
}
