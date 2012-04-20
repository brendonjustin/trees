#ifndef _TRIANGLE_H
#define _TRIANGLE_H

#include "edge.h"

class Material;

// ===========================================================
// Stores the indices to the 3 vertices of the triangles, 
// used by the mesh class

class Triangle {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  Triangle() {
    edge = NULL; 
    id = next_triangle_id;
    next_triangle_id++;
  }

  // =========
  // ACCESSORS
  Vertex* operator[](int i) const { 
    assert (edge != NULL);
    if (i==0) return edge->getStartVertex();
    if (i==1) return edge->getNext()->getStartVertex();
    if (i==2) return edge->getNext()->getNext()->getStartVertex();
    assert(0); exit(0);
  }
  Edge* getEdge() { 
    assert (edge != NULL);
    return edge; 
  }
  void setEdge(Edge *e) {
    assert (edge == NULL);
    edge = e;
  }
  Material* getMaterial()
  {
    assert (material != NULL);
    return material;
  }
  void setMaterial(Material* m) {material = m;}
  int getID() { return id; }
  double get_s(int vert) const {return s[vert];}
  double get_t(int vert) const {return t[vert];}
  void setTextureCoordinates(int vert, double _s, double _t)
  {
    assert(vert > -1 && vert < 3);
    s[vert] = _s; t[vert] = _t;
  }

  // NOTE: If you want to modify a triangle, it is recommended that
  // you remove it from the mesh, delete it, create a triangle object
  // with the changes, and re-add it.  This will ensure the edges get
  // updated appropriately.

protected:

  // don't use these constructors
  Triangle(const Triangle &/*t*/) { assert(0); exit(0); }
  Triangle& operator= (const Triangle &/*t*/) { assert(0); exit(0); }
  
  // ==============
  // REPRESENTATION
  Edge *edge;
  int id;

  //Texture coordinates
  //s,t[0] are texture coordinates at edge->getStartVertex()
  //s,t[1] are texture coordinates at edge->getNext()->getStartVertex()
  //s,t[2] are texture coordinates at edge->getNext()->getNext()->getStartVertex()
  double s[3],t[3];
  
  // triangles are indexed starting at 0
  static int next_triangle_id;

  //The material of this triangle
  Material *material;
};

// ===========================================================

#endif
