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
  double lambda;
  
public:
  Seeder(double expectedNum) : lambda(expectedNum) {};
  void getLocations(float xSize, float ySize, float gridSize, std::vector<std::pair<float, float> >& locs);
  
};

#endif
