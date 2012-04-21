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
  double rand;
  double sum = 0;
  int maxK = 10;
  int i = 0;
  
  for (int a = 0; a < area / blockSize; ++a) {
    rand = GLOBAL_mtrand.rand();
    sum = 0;
    for (i = 0; i < maxK; ++i) {
      //  two-dimensional poisson distribution substitutes lambda*area
      //  everywhere lambda appears
      sum += pow(m_lambda, i)*exp(-m_lambda) / factorial[i];
      
      if (sum > rand) {
        break;
      }
    }
    numPerBlock.push_back(i);
  }
}
