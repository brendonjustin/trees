/*
  -----Triangle Class Impementation-----
  Auston Sterling
  Brendon Justin

  Contains the implementation of the Triangle class.
*/

#include "triangle.h"
#include "ray.h"
#include "hit.h"
#include "vertex.h"
#include "matrix.h"

//Checks to see if the input Ray intersects this Triangle
bool Triangle::intersect(const Ray & r, Hit & h) const
{
  //Find the three vertices which make up this triangle
  Vertex* a = edge->getStartVertex();
  Vertex* b = edge->getNext()->getStartVertex();
  Vertex* c = edge->getNext()->getNext()->getStartVertex();

  Hit h2 = Hit(h);
  plane_intersect(r, h2);
  
  // figure out the barycentric coordinates:
  Vec3f Ro = r.getOrigin();
  Vec3f Rd = r.getDirection();
  // [ ax-bx   ax-cx  Rdx ][ beta  ]     [ ax-Rox ] 
  // [ ay-by   ay-cy  Rdy ][ gamma ]  =  [ ay-Roy ] 
  // [ az-bz   az-cz  Rdz ][ t     ]     [ az-Roz ] 
  // solve for beta, gamma, & t using Cramer's rule
  
  double detA = Matrix::det3x3(a->getPos().x()-b->getPos().x(),a->getPos().x()-c->getPos().x(),Rd.x(),
			       a->getPos().y()-b->getPos().y(),a->getPos().y()-c->getPos().y(),Rd.y(),
			       a->getPos().z()-b->getPos().z(),a->getPos().z()-c->getPos().z(),Rd.z());
  
  if (fabs(detA) <= 0.000001) return 0;
  assert (fabs(detA) >= 0.000001);

  double beta  = Matrix::det3x3(a->getPos().x()-Ro.x(),a->getPos().x()-c->getPos().x(),Rd.x(),
				a->getPos().y()-Ro.y(),a->getPos().y()-c->getPos().y(),Rd.y(),
				a->getPos().z()-Ro.z(),a->getPos().z()-c->getPos().z(),Rd.z()) / detA;
  
  double gamma = Matrix::det3x3(a->getPos().x()-b->getPos().x(),a->getPos().x()-Ro.x(),Rd.x(),
				a->getPos().y()-b->getPos().y(),a->getPos().y()-Ro.y(),Rd.y(),
				a->getPos().z()-b->getPos().z(),a->getPos().z()-Ro.z(),Rd.z()) / detA;

  if (beta >= -0.00001 && beta <= 1.00001 &&
      gamma >= -0.00001 && gamma <= 1.00001 &&
      beta + gamma <= 1.00001) {
    h = h2;
    // interpolate the texture coordinates
    double alpha = 1 - beta - gamma;
    double t_s = alpha * s[0] + beta * s[1] + gamma * s[2];
    double t_t = alpha * t[0] + beta * t[1] + gamma * t[2];
    h.setTextureCoords(t_s,t_t);
    assert (h.getT() >= 0.0001);
    return 1;
  }

  return 0;
}

bool Triangle::plane_intersect(const Ray &r, Hit &h) const
{
  // insert the explicit equation for the ray into the implicit equation of the plane
  
  // equation for a plane
  // ax + by + cz = d;
  // normal . p + direction = 0
  // plug in ray
  // origin + direction * t = p(t)
  // origin . normal + t * direction . normal = d;
  // t = d - origin.normal / direction.normal;

  Vec3f normal = computeNormal();
  double d = normal.Dot3((*this)[0]->getPos());

  double numer = d - r.getOrigin().Dot3(normal);
  double denom = r.getDirection().Dot3(normal);

  if (denom == 0) return 0;  // parallel to plane

  double t = numer / denom;
  if (t > 0.0001 && t < h.getT()) {
    h.set(t,this->getMaterial(),normal);
    assert (h.getT() >= 0.0001);
    return 1;
  }
  return 0;
}

Vec3f Triangle::computeNormal()
{
  Vec3f a = edge->getStartVertex()->getPos();
  Vec3f b = edge->getNext()->getStartVertex()->getPos();
  Vec3f c = edge->getNext()->getNext()->getStartVertex()->getPos();
  
  Vec3f v12 = b;
  v12 -= a;
  Vec3f v23 = c;
  v23 -= b;
  Vec3f normal;
  Vec3f::Cross3(normal,v12,v23);
  normal.Normalize();
  return normal;
}

Vec3f Triangle::computeNormal() const
{
  Vec3f a = edge->getStartVertex()->getPos();
  Vec3f b = edge->getNext()->getStartVertex()->getPos();
  Vec3f c = edge->getNext()->getNext()->getStartVertex()->getPos();
  
  Vec3f v12 = b;
  v12 -= a;
  Vec3f v23 = c;
  v23 -= b;
  Vec3f normal;
  Vec3f::Cross3(normal,v12,v23);
  normal.Normalize();
  return normal;
}
