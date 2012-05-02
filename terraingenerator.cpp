//
//  terraingenerator.cpp
//  trees
//
//  Created by Brendon Justin on 4/29/12.
//  Copyright (c) 2012 Brendon Justin. All rights reserved.
//

#include "terraingenerator.h"

#include "utils.h"

#include <cmath>

float TerrainGenerator::ratio = 1.0f;
float TerrainGenerator::scale = 1.0f;

float TerrainGenerator::getRandomOffset(int depth)
{
  float scaleDown = 1;

  for (int i = 0; i < depth; ++i)
  {
    scaleDown *= pow(2,-ratio);
  }
  if (GLOBAL_mtrand.rand() > 0.5) scaleDown = -scaleDown;
  return GLOBAL_mtrand.rand()*scale*scaleDown;
}

void TerrainGenerator::diamondIteration(std::vector<std::vector<float> >& vec, int count)
{
  int sideLength = vec.size();
  int sideLengthZero = sideLength - 1;
  int numSegments = pow(2, count-1);
  int span = sideLengthZero / numSegments;
  int halfSpan = span / 2;
  int x1, x2, y1, y2;
  float avg;

  for (int x = 0; x < sideLengthZero; x += span)
  {
    for (int y = 0; y < sideLengthZero; y += span)
    {
      x1 = x;
      x2 = x+span;
      y1 = y;
      y2 = y+span;

      avg = vec[x1][y1] + vec[x2][y1] + vec[x2][y2] + vec[x1][y2];
      avg *= 0.25f;
      vec[x1+halfSpan][y1+halfSpan] = avg + getRandomOffset(count);
    }
  }
}

void TerrainGenerator::squareIteration(std::vector<std::vector<float> >& vec, int count)
{
  int sideLength = vec.size();
  int sideLengthZero = sideLength - 1;
  int numSegments = pow(2, count-1);
  int span = sideLengthZero / numSegments;
  int halfSpan = span / 2;
  int x1, x2, y1, y2, xHalf, yHalf, rr, dd, ll, uu;
  float avg;

  for (int x = 0; x < sideLengthZero; x += span)
  {
    for (int y = 0; y < sideLengthZero; y += span)
    {
      x1 = x;
      x2 = x+span;
      y1 = y;
      y2 = y+span;
      xHalf = x+halfSpan;
      yHalf = y+halfSpan;

      rr = x + span + halfSpan;
      if (rr > sideLengthZero) rr = halfSpan;

      dd = y + span + halfSpan;
      if (dd > sideLengthZero) dd = halfSpan;

      ll = x - halfSpan;
      if (ll < 0) ll = sideLengthZero - halfSpan;

      uu = y - halfSpan;
      if (uu < 0) uu = sideLengthZero - halfSpan;

      avg = vec[x1][y1] + vec[xHalf][yHalf] + vec[x1][y2] + vec[ll][yHalf];
      avg *= 0.25f;
      vec[x1][yHalf] = avg + getRandomOffset(count);

      avg = vec[x1][y1] + vec[xHalf][uu] + vec[x2][y1] + vec[xHalf][yHalf];
      avg *= 0.25f;
      vec[xHalf][y1] = avg + getRandomOffset(count);

      avg = vec[x2][y1] + vec[rr][yHalf] + vec[x2][y2] + vec[xHalf][yHalf];
      avg *= 0.25f;
      vec[x2][yHalf] = avg + getRandomOffset(count);

      avg = vec[x1][y2] + vec[xHalf][yHalf] + vec[x2][y2] + vec[xHalf][dd];
      avg *= 0.25f;
      vec[xHalf][y2] = avg + getRandomOffset(count);
    }
  }
  
  //  Set the heights to be equal at the right and left edges,
  //  as well as the top and bottom edges.
  for (int x = 0; x < sideLength; x += span)
  {
    vec[x][sideLengthZero] = vec[x][0];
  }
  for (int y = 0; y < sideLength; y += span)
  {
    vec[sideLengthZero][y] = vec[0][y];
  }
}

//	Generate fractal terrain using the diamond-square algorithm
std::vector<std::vector<float> > TerrainGenerator::generate(int squaresPerSide)
{
  int count, iterations, pointsPerSide;
  int x1, x2, y1, y2;
  pointsPerSide = squaresPerSide + 1;
  std::vector<std::vector<float> > heights;

  for (int i = 0; i < pointsPerSide; ++i)
  {
    std::vector<float> thisVec (pointsPerSide, 0);
    heights.push_back(thisVec);
  }

  x1 = 0;
  y1 = 0;
  x2 = squaresPerSide;
  y2 = squaresPerSide;

  //  Initialize the corners to height 0
  heights[x1][y1] = heights[x1][y2] = heights[x2][y2] = heights[x2][y1] = 0;

  count = 0;
  iterations = log(heights.size() - 1) / log(2);
  while (count++ < iterations) {
    diamondIteration(heights, count);
    squareIteration(heights, count);
  }

  return heights;
}
