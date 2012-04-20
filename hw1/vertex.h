#ifndef _VERTEX_H
#define _VERTEX_H

#include "vectors.h"

// ==========================================================
// Stores th vertex position, used by the Mesh class

class Vertex {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Vertex(int i, const Vec3f &pos) : position(pos) { index = i; }
  
  // =========
  // ACCESSORS
  int getIndex() const { return index; }
  double x() const { return position.x(); }
  double y() const { return position.y(); }
  double z() const { return position.z(); }
  const Vec3f& getPos() const { return position; }

  // =========
  // MODIFIERS
  void setPos(Vec3f v) { position = v; }

private:

  // don't use these constructors
  Vertex() { assert(0); exit(0); }
  Vertex(const Vertex&) { assert(0); exit(0); }
  Vertex& operator=(const Vertex&) { assert(0); exit(0); }
  
  // ==============
  // REPRESENTATION
  Vec3f position;

  // this is the index from the original .obj file.
  // technically not part of the half-edge data structure, 
  // but we use it for hashing
  int index;  

  // NOTE: the vertices don't know anything about adjacency.  In some
  // versions of this data structure they have a pointer to one of
  // their incoming edges.  However, this data is very complicated to
  // maintain during mesh manipulation, so it has been omitted.

};

// ==========================================================

#endif

