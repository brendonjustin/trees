#include "forest.h"

#include "argparser.h"
#include "terraingenerator.h"

#include <numeric>

Forest::~Forest() {
	cleanupVBOs();
}

void Forest::initializeVBOs() {
  // create a pointer for the vertex & index VBOs
  glGenBuffers(1, &mesh_tri_verts_VBO[0]);
  glGenBuffers(1, &mesh_tri_indices_VBO[0]);
  glGenBuffers(1, &mesh_tri_texcoords_VBO[0]);
  setupVBOs();
}

void Forest::setupVBOs() {
  //  Setup the ground and the trees
  int numBlocks, numTrees = args->trees;
  //  Properties of the ground
  float area, sideLength;
  //  Area-based weights for vertices, to determine tree heights
  float a1, a2, a3, a4;

  //  The vertices for the squares used for the ground and trees
  Vec3f aT, bT, cT, dT, treeNormal;
  Vec3f aG, bG, cG, dG, gndNormal;
  Vec3f baseOffset, offset, hVec;
  Vec3f baseTreeLoc, treeHeight;

  VBOTriVert* forest_tri_verts;
  VBOTri* forest_tri_indices;
  VBOTriVert* gnd_mesh_tri_verts;
  VBOTri* gnd_mesh_tri_indices;

  //  Ground vertex heights and the locations of trees
  std::vector<std::vector<float> > heights;
  std::vector<std::vector<Vec3f> > treeLocations;

  Seeder seeder = Seeder(2);
  
  //  Note: numBlocks must be a power of two, squared, i.e. (2^x)^2
  //  Area need not be an integer value
  numBlocks = pow(pow(2, 7), 2);
  area = numBlocks;
  sideLength = sqrt(area / numBlocks);
  
  treeNormal = Vec3f(0, 0, 1);
  gndNormal = Vec3f(0, 1, 0);
  
  //  A variable sized vertical square with the bottom left corner at 0,0
  //  For drawing trees
  aT = Vec3f(0,               0,  0);
  bT = Vec3f(treeSize, treeSize,  0);
  cT = Vec3f(treeSize,        0,  0);
  dT = Vec3f(0,        treeSize,  0);

  //  A variable sized horizontal square with the bottom left corner at 0,0
  //  For drawing the ground
  aG = Vec3f(0,            0,  0);
  bG = Vec3f(sideLength,   0,  sideLength);
  cG = Vec3f(sideLength,   0,  0);
  dG = Vec3f(0,            0,  sideLength);
  hVec = Vec3f(0,1,0);
  
  //  Tweak some optional parameters and generate terrain heights
//    TerrainGenerator::setRatio(0.5f);
//    TerrainGenerator::setRatio(1.5f);
  TerrainGenerator::setRatio(2.0f);
//    TerrainGenerator::setRatio(2.5f);
//    TerrainGenerator::setScale(2.0f);
  TerrainGenerator::setScale(100.0f);
  heights = TerrainGenerator::generate((int)sqrt(numBlocks));

  treeLocations = seeder.getTreeLocations(area, numBlocks);
  
  forest_tri_verts = new VBOTriVert[numTrees*4];
  forest_tri_indices = new VBOQuad[numTrees];
  gnd_mesh_tri_verts = new VBOTriVert[numBlocks*4];
  gnd_mesh_tri_indices = new VBOTri[numBlocks*2];
  
  //  Draw ground squares and trees
  int locCounter = 0;
  int countTrees = 0;
  int blockNumber = 0;
  for (int i = 0; i < sqrt(numBlocks); ++i) {
    baseOffset = c*i;
    for (int j = 0; j < sqrt(numBlocks); ++j) {
      offset = baseOffset + d*j;
      treeLoc = Vec3f(0,0,0);

      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + aG + hVec*heights[i][j], normal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + bG + hVec*heights[i+1][j+1], normal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + cG + hVec*heights[i+1][j], normal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + dG + hVec*heights[i][j+1], normal);
      gnd_mesh_tri_indices[locCounter / 2 - 2] = VBOTri(locCounter - 4, locCounter - 3, locCounter - 2);
      gnd_mesh_tri_indices[locCounter / 2 - 1] = VBOTri(locCounter - 3, locCounter - 4, locCounter - 1);

      blockNumber = i*sqrt(numBlocks) + j;
      for (int k = 0; k < treeLocations[blockNumber].size(); ++k)
      {
        treeLoc = treeLocations[countTrees][k];
        treeHeight = (treeLoc.x - gnd_mesh_tri_verts[locCounter-1].x).length()*(treeLoc.z - gnd_mesh_tri_verts[locCounter-1].z).length() * gnd_mesh_tri_verts[locCounter-1].y;
        treeHeight += (treeLoc.x - gnd_mesh_tri_verts[locCounter-2].x).length()*(treeLoc.z - gnd_mesh_tri_verts[locCounter-2].z).length() * gnd_mesh_tri_verts[locCounter-2].y;
        treeHeight += (treeLoc.x - gnd_mesh_tri_verts[locCounter-3].x).length()*(treeLoc.z - gnd_mesh_tri_verts[locCounter-3].z).length() * gnd_mesh_tri_verts[locCounter-3].y;
        treeHeight += (treeLoc.x - gnd_mesh_tri_verts[locCounter-4].x).length()*(treeLoc.z - gnd_mesh_tri_verts[locCounter-4].z).length() * gnd_mesh_tri_verts[locCounter-4].y;
        treeHeight = hvec*treeHeight.y;
        treeLoc += treeHeight;

        forest_tri_verts[countTrees*4] = VBOTriVert(treeLoc + aT, treeNormal);
        forest_tri_verts[countTrees*4 + 1] = VBOTriVert(treeLoc + bT, treeNormal);
        forest_tri_verts[countTrees*4 + 2] = VBOTriVert(treeLoc + cT, treeNormal);
        forest_tri_verts[countTrees*4 + 3] = VBOTriVert(treeLoc + dT, treeNormal);
        
        forest_tri_indices[countTrees++] = VBOTri(locCounter - 4, locCounter - 3, locCounter - 2, locCounter - 1);
      }
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER,gnd_mesh_tri_verts_VBO);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(VBOTriVert) * numBlocks * 4,
               gnd_mesh_tri_verts,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,gnd_mesh_tri_indices_VBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(VBOTri) * numBlocks * 2,
               gnd_mesh_tri_indices,
               GL_STATIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER,forest_tri_verts_VBO);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(VBOTriVert) * numBlocks * 4,
               forest_tri_verts,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,forest_tri_indices_VBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(VBOQuad) * numBlocks * 2,
               forest_tri_indices,
               GL_STATIC_DRAW);
  
  delete [] gnd_mesh_tri_verts;
  delete [] gnd_mesh_tri_indices;
  delete [] forest_tri_verts;
  delete [] forest_tri_indices;

  num_gnd_tris = numBlocks * 2;
  num_trees = numTrees;
}

void Mesh::cleanupVBOs() {
  glDeleteBuffers(1, &forest_tri_verts_VBO);
  glDeleteBuffers(1, &forest_tri_indices_VBO);
  glDeleteBuffers(1, &forest_tri_texcoords_VBO);
  glDeleteBuffers(1, &gnd_mesh_tri_verts_VBO);
  glDeleteBuffers(1, &gnd_mesh_tri_indices_VBO);
  glDeleteBuffers(1, &gnd_mesh_verts_VBO);
}

void Mesh::drawVBOs() {
  // draw the ground and the trees

  //  Ground
  glEnable(GL_LIGHTING);
  glColor3f(0,1,0);
  glBindBuffer(GL_ARRAY_BUFFER, gnd_mesh_tri_verts_VBO);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(0));
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(12));
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gnd_mesh_tri_indices_VBO);
  glDrawElements(GL_TRIANGLES,
                 num_gnd_tris*3,
                 GL_UNSIGNED_INT,
                 BUFFER_OFFSET(0));
  
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  //  Trees
  glBindBuffer(GL_ARRAY_BUFFER, forest_tri_verts_VBO);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(0));
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(12));
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, forest_tri_indices_VBO);
  glDrawElements(GL_QUADS,
                 num_trees*4,
                 GL_UNSIGNED_INT,
                 BUFFER_OFFSET(0));
  
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}

/*
glBindTexture(GL_TEXTURE_2D, hemisphere->getNearestView(Vec3f(0,0,0),camera->getPosition())->textureID());
*/