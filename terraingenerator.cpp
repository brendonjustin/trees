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

float TerrainGenerator::ratio;

float TerrainGenerator::getRandomOffset(int depth)
{
  return (2*GLOBAL_mtrand.rand() - 1)*pow(2,-ratio);
}

void TerrainGenerator::diamondIteration(std::vector<float>& vec, int count)
{
  int sideLength = sqrt(vec.size());
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

      avg = vec[x1 + y1*sideLength] + vec[x2 + y1*sideLength] + vec[x2 + y2*sideLength] + vec[x1 + y2*sideLength];
      avg *= 0.25f;
      vec[x1+halfSpan+sideLength*(y1+halfSpan)] = avg + getRandomOffset(count);
    }
  }
}

void TerrainGenerator::squareIteration(std::vector<float>& vec, int count)
{
  int sideLength = sqrt(vec.size());
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

      avg = vec[x1 + y1*sideLength] + vec[xHalf + yHalf*sideLength] + vec[x1 + y2*sideLength] + vec[ll + yHalf*sideLength];
      avg *= 0.25f;
      vec[x1 + yHalf*sideLength] = avg + getRandomOffset(count);

      avg = vec[x1 + y1*sideLength] + vec[xHalf + uu*sideLength] + vec[x2 + y1*sideLength] + vec[xHalf + yHalf*sideLength];
      avg *= 0.25f;
      vec[xHalf + y1*sideLength] = avg + getRandomOffset(count);

      avg = vec[x2 + y1*sideLength] + vec[rr + yHalf*sideLength] + vec[x2 + y2*sideLength] + vec[xHalf + yHalf*sideLength];
      avg *= 0.25f;
      vec[x2 + yHalf*sideLength] = avg + getRandomOffset(count);

      avg = vec[x1 + y2*sideLength] + vec[xHalf + yHalf*sideLength] + vec[x2 + y2*sideLength] + vec[xHalf + dd*sideLength];
      avg *= 0.25f;
      vec[xHalf + y2*sideLength] = avg + getRandomOffset(count);
    }
  }

  //  Set the heights to be equal at the right and left edges,
  //  as well as the top and bottom edges.
  for (int x = 0; x < sideLengthZero; x += span)
  {
    vec[x + sideLengthZero*sideLength] = vec[x];
  }
  for (int y = 0; y < sideLengthZero; y += span)
  {
    vec[sideLengthZero + sideLengthZero*y] = vec[sideLengthZero*y];
  }
}

//	Generate fractal terrain using the diamond-square algorithm
std::vector<float> TerrainGenerator::generate(int squaresPerSide)
{
  int count, iterations;
  int pt1, pt2, pt3, pt4;
  int x1, x2, x3, x4;
  int y1, y2, y3, y4;
  std::vector<float> heights (squaresPerSide*squaresPerSide);

  x1 = 0;
  y1 = 0;
  x2 = squaresPerSide-1;
  y2 = 0;
  x3 = 0;
  y3 = squaresPerSide-1;
  x4 = squaresPerSide-1;
  y4 = squaresPerSide-1;

  pt1 = x1 + y1*squaresPerSide;
  pt2 = x2 + y2*squaresPerSide;
  pt3 = x3 + y3*squaresPerSide;
  pt4 = x4 + y4*squaresPerSide;

  //  Initialize the corners to height 0
  heights[pt1] = heights[pt2] = heights[pt3] = heights[pt4] = 0;

  count = 0;
  iterations = log(squaresPerSide - 1) / log(2);
  while (count++ < iterations) {
    diamondIteration(heights, count);
    squareIteration(heights, count);
  }

  return heights;
}
