#ifndef _RAY_H
#define _RAY_H

#include <iostream>
#include "vectors.h"

// Ray class mostly copied from Peter Shirley and Keith Morley
// ====================================================================
// ====================================================================

class Ray {

public:

  // CONSTRUCTOR & DESTRUCTOR
  Ray (const Vec3f &orig, const Vec3f &dir) {
    origin = orig; 
    direction = dir; }

  // ACCESSORS
  const Vec3f& getOrigin() const { return origin; }
  const Vec3f& getDirection() const { return direction; }
  Vec3f pointAtParameter(double t) const {
    return origin+direction*t; }

private:
  Ray () { assert(0); } // don't use this constructor

  // REPRESENTATION
  Vec3f origin;
  Vec3f direction;
};

inline std::ostream &operator<<(std::ostream &os, const Ray &r) {
  os << "Ray <" <<r.getOrigin()<<", "<<r.getDirection()<<">";
  return os;
}

// ====================================================================
// ====================================================================

#endif
