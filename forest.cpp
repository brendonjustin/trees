#include "forest.h"
#include "seeder.h"

#include "argparser.h"
#include "hemisphere.h"
#include "matrix.h"
#include "mesh.h"
#include "terraingenerator.h"
#include "view.h"

#include <iostream>

// helper for VBOs
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Forest::Forest(ArgParser *a, Hemisphere *h) : args(a), hemisphere(h), 
                                              num_trees(0), tree_size(5),
                                              tree_buffer_set(false) {

  //  Note: num_blocks must be a power of two, squared, i.e. (2^x)^2
  num_blocks = pow(pow(2, 4), 2);
  area = num_blocks * tree_size * 6;
  
  //  For drawing trees
  //  A variable sized vertical square with the bottom center at 0,0
  aT = Vec3f(-tree_size / 2.0f, 0, 0);
  bT = Vec3f(tree_size / 2.0f,  tree_size,  0);
  cT = Vec3f(tree_size / 2.0f,  0,  0);
  dT = Vec3f(-tree_size / 2.0f, tree_size,  0);

  Seeder seeder = Seeder(2);
  //  Get the block-space coordinates of each tree
  tree_locations = seeder.getTreeLocations(area, num_blocks, tree_size);
  for (unsigned int i = 0; i < tree_locations.size(); ++i)
  {
    num_trees += tree_locations[i].size();
  }
  forest_quad_verts_VBO = std::vector<GLuint> (num_trees);
  forest_quad_indices_VBO = std::vector<GLuint> (num_trees);
  forest_quad_texcoords_VBO = std::vector<GLuint> (num_trees);
  forest_quad_textures = std::vector<GLuint> (num_trees);
}

Forest::~Forest() {
	cleanupVBOs();
}

void Forest::initializeVBOs() {
  forest_quad_verts = new VBOTriVert[num_trees*4];
  
  // create a pointer for the vertex & index VBOs
  glGenBuffers(num_trees, &forest_quad_verts_VBO[0]);
  glGenBuffers(num_trees, &forest_quad_indices_VBO[0]);
  glGenBuffers(num_trees, &forest_quad_texcoords_VBO[0]);
  // for (int i = 0; i < num_trees; ++i)
  // {
  //   glGenBuffers(1, &forest_quad_verts_VBO[i]);
  //   glGenBuffers(1, &forest_quad_indices_VBO[i]);
  //   glGenBuffers(1, &forest_quad_texcoords_VBO[i]);
  // }
  glGenBuffers(1, &gnd_mesh_tri_verts_VBO);
  glGenBuffers(1, &gnd_mesh_tri_indices_VBO);
  glGenBuffers(1, &gnd_mesh_verts_VBO);
  // glGenTextures(num_trees, &forest_quad_textures);
  setupVBOs();
}

void Forest::setupVBOs() {
  //  Setup the ground and the trees
  //  Properties of the ground
  float blockSideLength;
  //  Area-based weights for vertices, to determine tree heights
  float treeHeight, a1, a2, a3, a4, sumA;
  int sqrtNumBlocks;

  //  The vertices for the squares used for the ground and trees
  Vec3f aG, bG, cG, dG, gndNormal;
  Vec3f baseOffset, blockOffset, hVec;
  Vec3f treeLocation;

  VBOQuad* forest_quad_indices;
  VBOTex* forest_quad_texcoords;
  VBOTriVert* gnd_mesh_tri_verts;
  VBOTri* gnd_mesh_tri_indices;

  //  Ground vertex heights
  std::vector<std::vector<float> > heights;
  
  blockSideLength = sqrt(area / num_blocks);

  gndNormal = Vec3f(0, 1, 0);

  //  For drawing the ground
  //  A variable sized horizontal square with the bottom left corner at 0,0
  aG = Vec3f(0,               0,  0);
  bG = Vec3f(blockSideLength, 0,  blockSideLength);
  cG = Vec3f(blockSideLength, 0,  0);
  dG = Vec3f(0,               0,  blockSideLength);
  hVec = Vec3f(0,1,0);
  
  sqrtNumBlocks = sqrt(num_blocks);
  
  //  Tweak some optional parameters and generate terrain heights
  TerrainGenerator::setRatio(0.5f);
  // TerrainGenerator::setRatio(1.5f);
  // TerrainGenerator::setRatio(2.0f);
  // TerrainGenerator::setRatio(2.5f);
  // TerrainGenerator::setScale(2.0f);
  // TerrainGenerator::setScale(100.0f);
  TerrainGenerator::setScale(sqrt(sqrtNumBlocks));
  heights = TerrainGenerator::generate(sqrtNumBlocks);
  
  forest_quad_indices = new VBOQuad[num_trees];
  forest_quad_texcoords = new VBOTex[num_trees*4];

  gnd_mesh_tri_verts = new VBOTriVert[num_blocks*4];
  gnd_mesh_tri_indices = new VBOTri[num_blocks*2];
  
  //  Draw ground squares and trees
  int locCounter = 0;
  int countTrees = 0;
  countTrees = num_trees-1;
  int blockNumber = 0;
  for (int i = 0; i < sqrtNumBlocks; ++i) {
    baseOffset = cG*i;
    for (int j = 0; j < sqrtNumBlocks; ++j) {
      blockOffset = baseOffset + dG*j;

      //  Add a ground square
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(blockOffset + aG + hVec*heights[i][j], gndNormal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(blockOffset + bG + hVec*heights[i+1][j+1], gndNormal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(blockOffset + cG + hVec*heights[i+1][j], gndNormal);
      gnd_mesh_tri_verts[locCounter++] = VBOTriVert(blockOffset + dG + hVec*heights[i][j+1], gndNormal);

      gnd_mesh_tri_indices[locCounter / 2 - 2] = VBOTri(locCounter - 4, locCounter - 3, locCounter - 2);
      gnd_mesh_tri_indices[locCounter / 2 - 1] = VBOTri(locCounter - 3, locCounter - 4, locCounter - 1);

      //  Create the trees in this ground square
      blockNumber = i*sqrtNumBlocks + j;
      for (unsigned int k = 0; k < tree_locations[blockNumber].size(); ++k)
      {
        treeLocation = blockOffset + tree_locations[blockNumber][k];

        //  Calculate the height that the tree should be at,
        //  by averaging nearby verticies' heights by proximity
        a1 = abs((treeLocation.x() - gnd_mesh_tri_verts[locCounter-1].x)*(treeLocation.z() - gnd_mesh_tri_verts[locCounter-1].z));
        a2 = abs((treeLocation.x() - gnd_mesh_tri_verts[locCounter-2].x)*(treeLocation.z() - gnd_mesh_tri_verts[locCounter-2].z));
        a3 = abs((treeLocation.x() - gnd_mesh_tri_verts[locCounter-3].x)*(treeLocation.z() - gnd_mesh_tri_verts[locCounter-3].z));
        a4 = abs((treeLocation.x() - gnd_mesh_tri_verts[locCounter-4].x)*(treeLocation.z() - gnd_mesh_tri_verts[locCounter-4].z));
        sumA = a1 + a2 + a3 + a4;
        a1 /= sumA;
        a2 /= sumA;
        a3 /= sumA;
        a4 /= sumA;
        
        treeHeight = gnd_mesh_tri_verts[locCounter-1].y * a1;
        treeHeight += gnd_mesh_tri_verts[locCounter-2].y * a2;
        treeHeight += gnd_mesh_tri_verts[locCounter-3].y * a3;
        treeHeight += gnd_mesh_tri_verts[locCounter-4].y * a4;
        treeLocation.sety(treeHeight);

        //  Save the world-space tree coordinate over the block-space coordinate
        tree_locations[blockNumber][k] = treeLocation;
        
        //  Create the quad to draw the tree on in another function later
        //  For now just set the vertex indices and texture coordinates
        forest_quad_indices[countTrees] = VBOQuad(locCounter - 4, locCounter - 3, locCounter - 2, locCounter - 1);
        forest_quad_texcoords[countTrees*4] = VBOTex(0,0);
        forest_quad_texcoords[countTrees*4 + 1] = VBOTex(0,1);
        forest_quad_texcoords[countTrees*4 + 2] = VBOTex(1,1);
        forest_quad_texcoords[countTrees*4 + 3] = VBOTex(1,0);

        //  Set the texture for this tree
        forest_quad_textures[countTrees] = hemisphere->getNearestView(treeLocation, camera_pos)->textureID();

        // ++countTrees;
        --countTrees;
      }
    }
  }
  
  setTreeQuads();

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
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,forest_quad_indices_VBO[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(VBOQuad) * num_trees,
               forest_quad_indices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,forest_quad_texcoords_VBO[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(VBOTex) * num_trees * 4,
               forest_quad_texcoords,
               GL_STATIC_DRAW);

  // for (int i = 0; i < num_trees; ++i)
  // {
  //   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,forest_quad_indices_VBO[i]);
  //   glBufferData(GL_ELEMENT_ARRAY_BUFFER,
  //                sizeof(VBOQuad),
  //                &forest_quad_indices[i],
  //                GL_STATIC_DRAW);
  //   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,forest_quad_texcoords_VBO[i]);
  //   glBufferData(GL_ELEMENT_ARRAY_BUFFER,
  //                sizeof(VBOTex) * 4,
  //                &forest_quad_texcoords[i],
  //                GL_STATIC_DRAW);
  // }
  
  delete [] gnd_mesh_tri_verts;
  delete [] gnd_mesh_tri_indices;

  delete [] forest_quad_indices;
  delete [] forest_quad_texcoords;

  num_gnd_tris = num_blocks * 2;
}

void Forest::cleanupVBOs() {
  delete [] forest_quad_verts;
  
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
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_LIGHTING );
  glColor3f(0,0.3f,0);
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
  glDisable( GL_LIGHTING );
  glDisable( GL_DEPTH_TEST );

  glEnable( GL_DEPTH_TEST );
  // glEnable( GL_MULTISAMPLE_ARB );
  // glEnable( GL_SAMPLE_ALPHA_TO_COVERAGE );
  glEnable( GL_TEXTURE_2D );
  glEnable( GL_BLEND );
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor3f(1.0,1.0,1.0);

  //  Trees
  bool many_textures = false;
  if (many_textures)
  {
    for (int i = 0; i < num_trees; ++i)
    {
      // glBindBuffer(GL_ARRAY_BUFFER, forest_quad_verts_VBO[i]);
      // glEnableClientState(GL_VERTEX_ARRAY);
      // glVertexPointer(3, GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(0));
      // glEnableClientState(GL_NORMAL_ARRAY);
      // glNormalPointer(GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(12));
      // glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      // glBindBuffer(GL_ARRAY_BUFFER, forest_quad_texcoords_VBO[i]);
      // glTexCoordPointer(2, GL_FLOAT, sizeof(VBOTex), BUFFER_OFFSET(0));
      // glBindTexture(GL_TEXTURE_2D, forest_quad_textures[i]);

      // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, forest_quad_indices_VBO[i]);
      // glDrawElements(GL_QUADS,
      //                4,
      //                GL_UNSIGNED_INT,
      //                BUFFER_OFFSET(0));
      
      // glDisableClientState(GL_TEXTURE_COORD_ARRAY);
      // glDisableClientState(GL_NORMAL_ARRAY);
      // glDisableClientState(GL_VERTEX_ARRAY);
    }
  } else {
    glBindBuffer(GL_ARRAY_BUFFER, forest_quad_verts_VBO[0]);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(0));
    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, sizeof(VBOTriVert), BUFFER_OFFSET(12));
    glBindBuffer(GL_ARRAY_BUFFER, forest_quad_texcoords_VBO[0]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(VBOTex), BUFFER_OFFSET(0));
    glBindTexture(GL_TEXTURE_2D, forest_quad_textures[0]);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, forest_quad_indices_VBO[0]);
    glDrawElements(GL_QUADS,
                   num_trees * 4,
                   GL_UNSIGNED_INT,
                   BUFFER_OFFSET(0));
    
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
  }
  glDisable( GL_TEXTURE_2D );
  glDisable( GL_BLEND );
  // glDisable( GL_SAMPLE_ALPHA_TO_COVERAGE );
  // glDisable( GL_MULTISAMPLE_ARB );
  glDisable( GL_DEPTH_TEST );
}

void Forest::setCameraPosition(Vec3f cameraPos) {
  camera_pos = cameraPos;
}

void Forest::cameraMoved(Vec3f cameraPos) {
  setCameraPosition(cameraPos);
  setTreeQuads();
}

void Forest::setTreeQuads() {
  int counter = 0;
  for (unsigned int i = 0; i < tree_locations.size(); ++i)
  {
    for (unsigned int j = 0; j < tree_locations[i].size(); ++j)
    {
      Vec3f treeLoc = tree_locations[i][j];
      //  Create a quad to draw the tree on
      Vec3f center = treeLoc + Vec3f(0, tree_size/2, 0);
      Vec3f toCamera = camera_pos - center;
      toCamera.Normalize();
      Vec3f horiz, vert;
      Vec3f::Cross3(horiz, Vec3f(0,1,0), toCamera);
      Vec3f::Cross3(vert, toCamera, horiz);
      horiz.Normalize(); vert.Normalize();
      forest_quad_verts[counter*4] = VBOTriVert(center - (tree_size/2)*horiz - (tree_size/2)*vert, toCamera);
      forest_quad_verts[counter*4+1] = VBOTriVert(center - (tree_size/2)*horiz + (tree_size/2)*vert, toCamera);
      forest_quad_verts[counter*4+2] = VBOTriVert(center + (tree_size/2)*horiz + (tree_size/2)*vert, toCamera);
      forest_quad_verts[counter*4+3] = VBOTriVert(center + (tree_size/2)*horiz - (tree_size/2)*vert, toCamera);
      forest_quad_textures[counter++] = hemisphere->getNearestView(tree_locations[i][j], camera_pos)->textureID();
    }
  }
  
  glBindBuffer(GL_ARRAY_BUFFER,forest_quad_verts_VBO[0]);
  if (tree_buffer_set)
  {
    glBufferSubData(GL_ARRAY_BUFFER,
                    0,
                    sizeof(VBOTriVert) * num_trees * 4,
                    forest_quad_verts);
  }
  else
  {
    tree_buffer_set = true;
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(VBOTriVert) * num_trees * 4,
                 forest_quad_verts,
                 GL_STATIC_DRAW);
  }

  // if (tree_buffer_set)
  // {
  //   for (int i = 0; i < num_trees; ++i)
  //   {
  //     glBindBuffer(GL_ARRAY_BUFFER,forest_quad_verts_VBO[i]);
  //     glBufferSubData(GL_ARRAY_BUFFER,
  //                     0,
  //                     sizeof(VBOTriVert) * 4,
  //                     &forest_quad_verts[i]);
  //   }
  // }
  // else
  // {
  //   for (int i = 0; i < num_trees; ++i)
  //   {
  //     glBindBuffer(GL_ARRAY_BUFFER,forest_quad_verts_VBO[i]);
  //     glBufferData(GL_ARRAY_BUFFER,
  //                  sizeof(VBOTriVert) * 4,
  //                  &forest_quad_verts[i],
  //                  GL_STATIC_DRAW);
  //   }
  //   tree_buffer_set = true;
  // }
  
}
