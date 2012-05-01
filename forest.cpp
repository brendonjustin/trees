#include "forest.h"
#include "seeder.h"

#include "argparser.h"
#include "mesh.h"
#include "terraingenerator.h"

// helper for VBOs
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Forest::Forest(ArgParser *a, hemisphere *h) : args(a), hemisphere(h), tree_size(10) {
  num_trees = 0;

  //  Note: num_blocks must be a power of two, squared, i.e. (2^x)^2
  num_blocks = pow(pow(2, 7), 2);
  area = num_blocks;

  Seeder seeder = Seeder(2);
  tree_locations = seeder.getTreeLocations(area, num_blocks);
  for (int i = 0; i < tree_locations.size(); ++i)
  {
    num_trees += tree_locations[i].size();
  }
}

Forest::~Forest() {
	cleanupVBOs();
}

void Forest::initializeVBOs() {
  // create a pointer for the vertex & index VBOs
  glGenBuffers(num_trees, &forest_quad_verts_VBO[0]);
  glGenBuffers(num_trees, &forest_quad_indices_VBO[0]);
  glGenBuffers(num_trees, &forest_quad_texcoords_VBO[0]);
  glGenBuffers(1, &gnd_mesh_tri_verts_VBO);
  glGenBuffers(1, &gnd_mesh_tri_indices_VBO);
  glGenBuffers(1, &gnd_mesh_verts_VBO);
  setupVBOs();
}

void Forest::setupVBOs() {
  //  Setup the ground and the trees
  //  Properties of the ground
  float sideLength;
  //  Area-based weights for vertices, to determine tree heights
  float a1, a2, a3, a4;

  //  The vertices for the squares used for the ground and trees
  Vec3f aT, bT, cT, dT, treeNormal;
  Vec3f aG, bG, cG, dG, gndNormal;
  Vec3f baseOffset, offset, hVec;
  Vec3f baseTreeLoc, treeHeight, treeLoc;

  VBOTriVert* forest_quad_verts;
  VBOQuad* forest_quad_indices;
  VBOTriVert* gnd_mesh_tri_verts;
  VBOTri* gnd_mesh_tri_indices;

  //  Ground vertex heights and the locations of trees
  std::vector<std::vector<float> > heights;
  
  sideLength = sqrt(area / num_blocks);

  treeNormal = Vec3f(0, 0, 1);
  gndNormal = Vec3f(0, 1, 0);
  
  //  A variable sized vertical square with the bottom left corner at 0,0
  //  For drawing trees
  aT = Vec3f(0,                 0,  0);
  bT = Vec3f(tree_size, tree_size,  0);
  cT = Vec3f(tree_size,         0,  0);
  dT = Vec3f(0,         tree_size,  0);

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
  heights = TerrainGenerator::generate((int)sqrt(num_blocks));
  
  forest_quad_verts = new VBOTriVert[num_trees*4];
  forest_quad_indices = new VBOQuad[num_trees];
  gnd_mesh_tri_verts = new VBOTriVert[num_blocks*4];
  gnd_mesh_tri_indices = new VBOTri[num_blocks*2];
  
  //  Draw ground squares and trees
  int locCounter = 0;
  int countTrees = 0;
  int blockNumber = 0;
  for (int i = 0; i < sqrt(num_blocks); ++i) {
    baseOffset = cG*i;
    for (int j = 0; j < sqrt(num_blocks); ++j) {
      offset = baseOffset + dG*j;
      treeLoc = Vec3f(0,0,0);

      //  Add a ground square
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + aG + hVec*heights[i][j], gndNormal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + bG + hVec*heights[i+1][j+1], gndNormal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + cG + hVec*heights[i+1][j], gndNormal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(offset + dG + hVec*heights[i][j+1], gndNormal);
      gnd_mesh_tri_indices[locCounter / 2 - 2] = VBOTri(locCounter - 4, locCounter - 3, locCounter - 2);
      gnd_mesh_tri_indices[locCounter / 2 - 1] = VBOTri(locCounter - 3, locCounter - 4, locCounter - 1);

      //  Create the trees in this ground square
      blockNumber = i*sqrt(num_blocks) + j;
      for (int k = 0; k < tree_locations[blockNumber].size(); ++k)
      {
        treeLoc = tree_locations[countTrees][k];
        a1 = (treeLoc.x() - gnd_mesh_tri_verts[locCounter-1].x)*(treeLoc.z() - gnd_mesh_tri_verts[locCounter-1].z) * gnd_mesh_tri_verts[locCounter-1].y;
        a2 = (treeLoc.x() - gnd_mesh_tri_verts[locCounter-2].x)*(treeLoc.z() - gnd_mesh_tri_verts[locCounter-2].z) * gnd_mesh_tri_verts[locCounter-2].y;
        a3 = (treeLoc.x() - gnd_mesh_tri_verts[locCounter-3].x)*(treeLoc.z() - gnd_mesh_tri_verts[locCounter-3].z) * gnd_mesh_tri_verts[locCounter-3].y;
        a4 = (treeLoc.x() - gnd_mesh_tri_verts[locCounter-4].x)*(treeLoc.z() - gnd_mesh_tri_verts[locCounter-4].z) * gnd_mesh_tri_verts[locCounter-4].y;
        
        treeHeight = Vec3f(gnd_mesh_tri_verts[locCounter-1].x, gnd_mesh_tri_verts[locCounter-1].y, gnd_mesh_tri_verts[locCounter-1].z) * a1;
        treeHeight += Vec3f(gnd_mesh_tri_verts[locCounter-2].x, gnd_mesh_tri_verts[locCounter-1].y, gnd_mesh_tri_verts[locCounter-1].z) * a2;
        treeHeight += Vec3f(gnd_mesh_tri_verts[locCounter-3].x, gnd_mesh_tri_verts[locCounter-1].y, gnd_mesh_tri_verts[locCounter-1].z) * a3;
        treeHeight += Vec3f(gnd_mesh_tri_verts[locCounter-4].x, gnd_mesh_tri_verts[locCounter-1].y, gnd_mesh_tri_verts[locCounter-1].z) * a4;
        treeHeight = hVec*treeHeight.y();
        treeLoc += treeHeight;

        forest_quad_verts[countTrees*4] = VBOTriVert(treeLoc + aT, treeNormal);
        forest_quad_verts[countTrees*4 + 1] = VBOTriVert(treeLoc + bT, treeNormal);
        forest_quad_verts[countTrees*4 + 2] = VBOTriVert(treeLoc + cT, treeNormal);
        forest_quad_verts[countTrees*4 + 3] = VBOTriVert(treeLoc + dT, treeNormal);
        
        forest_quad_indices[countTrees++] = VBOQuad(locCounter - 4, locCounter - 3, locCounter - 2, locCounter - 1);
      }
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER,gnd_mesh_tri_verts_VBO);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(VBOTriVert) * num_blocks * 4,
               gnd_mesh_tri_verts,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,gnd_mesh_tri_indices_VBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(VBOTri) * num_blocks * 2,
               gnd_mesh_tri_indices,
               GL_STATIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER,forest_quad_verts_VBO[0]);
  glBufferData(GL_ARRAY_BUFFER,
               sizeof(VBOTriVert) * num_trees * 4,
               forest_quad_verts,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,forest_quad_indices_VBO[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(VBOQuad) * num_trees,
               forest_quad_indices,
               GL_STATIC_DRAW);
  
  delete [] gnd_mesh_tri_verts;
  delete [] gnd_mesh_tri_indices;
  delete [] forest_quad_verts;
  delete [] forest_quad_indices;

  num_gnd_tris = num_blocks * 2;
}

void Forest::cleanupVBOs() {
  glDeleteBuffers(num_trees, &forest_quad_verts_VBO[0]);
  glDeleteBuffers(num_trees, &forest_quad_indices_VBO[0]);
  glDeleteBuffers(num_trees, &forest_quad_texcoords_VBO[0]);
  glDeleteBuffers(1, &gnd_mesh_tri_verts_VBO);
  glDeleteBuffers(1, &gnd_mesh_tri_indices_VBO);
  glDeleteBuffers(1, &gnd_mesh_verts_VBO);
}

void Forest::drawVBOs() {
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
  glBindBuffer(GL_ARRAY_BUFFER, forest_quad_verts_VBO[0]);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(0));
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(12));
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, forest_quad_indices_VBO[0]);
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