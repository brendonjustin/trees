#ifndef _BOUNDING_BOX_H_
#define _BOUNDING_BOX_H_

#include <cassert>
#include <algorithm>
#include "vectors.h"

// ====================================================================
// ====================================================================

class BoundingBox {

public:

  // ========================
  // CONSTRUCTOR & DESTRUCTOR
  BoundingBox() { Set(Vec3f(0,0,0),Vec3f(0,0,0)); }
  BoundingBox(Vec3f _minimum, Vec3f _maximum) { Set(_minimum,_maximum); }

  // =========
  // ACCESSORS
  void Get(Vec3f &_minimum, Vec3f &_maximum) const {
    _minimum = minimum;
    _maximum = maximum; }
  Vec3f getMin() const { return minimum; }
  Vec3f getMax() const { return maximum; }
  void getCenter(Vec3f &c) const {
    c = maximum; 
    c -= minimum;
    c *= 0.5f;
    c += minimum;
  }
  double maxDim() const {
    double x = maximum.x() - minimum.x();
    double y = maximum.y() - minimum.y();
    double z = maximum.z() - minimum.z();
#if defined(_WIN32) 
    // windows already has them defined...
    return max(x,max(y,z));
#else
    return std::max(x,std::max(y,z));
#endif
  }

  // =========
  // MODIFIERS
  void Set(BoundingBox *bb) {
    assert (bb != NULL);
    minimum = bb->minimum;
    maximum = bb->maximum; }
  void Set(Vec3f _minimum, Vec3f _maximum) {
    assert (minimum.x() <= maximum.x() &&
	    minimum.y() <= maximum.y() &&
	    minimum.z() <= maximum.z());
    minimum = _minimum;
    maximum = _maximum; }
  void Extend(const Vec3f v) {
#if defined(_WIN32) 
    // windows already has them defined...
    minimum = Vec3f(min(minimum.x(),v.x()),
		    min(minimum.y(),v.y()),
		    min(minimum.z(),v.z()));
    maximum = Vec3f(max(maximum.x(),v.x()),
		    max(maximum.y(),v.y()),
		    max(maximum.z(),v.z())); 
#else
    minimum = Vec3f(std::min(minimum.x(),v.x()),
		    std::min(minimum.y(),v.y()),
		    std::min(minimum.z(),v.z()));
    maximum = Vec3f(std::max(maximum.x(),v.x()),
		    std::max(maximum.y(),v.y()),
		    std::max(maximum.z(),v.z())); 
#endif
  }
  void Extend(BoundingBox *bb) {
    assert (bb != NULL);
    Extend(bb->minimum);
    Extend(bb->maximum); }

  // =========
  // DEBUGGING 
  void Print(const char *s="") const {
    printf ("BOUNDING BOX %s: %f %f %f  -> %f %f %f\n", s,
            minimum.x(),minimum.y(),minimum.z(),
            maximum.x(),maximum.y(),maximum.z()); }

private:

  // ==============
  // REPRESENTATION
  Vec3f minimum;
  Vec3f maximum;
};

// ====================================================================
// ====================================================================

#endif
