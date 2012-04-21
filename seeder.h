//
//  seeder.h
//  trees
//
//  Created by Brendon Justin on 4/19/12.
//  Copyright (c) 2012 Naga Softworks, LLC. All rights reserved.
//

#ifndef trees_seeder_h
#define trees_seeder_h

#include <vector>
#include <utility>  //  pair

class Seeder {
  double m_lambda;
  static const int factorial[];
  
public:
  Seeder(double expectedNum) : m_lambda(expectedNum) {};
  void getDistribution(float area, float blockSize, std::vector<int>& numPerBlock);
  
};

#endif
