#ifndef _UTILS_H
#define _UTILS_H

#include <cmath>
#include "vectors.h"
#include "MersenneTwister.h"

// a single random number generate for reproduceable randomness
extern MTRand GLOBAL_mtrand;

// =========================================================================
// EPSILON is a necessary evil for raytracing implementations
// The appropriate value for epsilon depends on the precision of
// the floating point calculations on your hardware **AND** on the
// overall dimensions of the scene and your camera projection matrix.
#define EPSILON 0.0001


// =========================================================================
// These two functions convert between linear intensity values
// (approximate range 0->1) to an sRGB value (approximate range 0->1).
// The sRGB values make best use of 8 bit storage and are the best
// input for most displays and darkened viewing environments.

#define SRGB_ALPHA 0.055

inline double linear_to_srgb(double x) {
  double answer;
  if (x <= 0.0031308)
    answer = 12.92*x;
  else 
    answer = (1+SRGB_ALPHA)*(pow(x,1/2.4)-SRGB_ALPHA);
  return answer;
}

inline double srgb_to_linear(double x) {
  double answer;
  if (x <= 0.04045)
    answer = x/12.92;
  else 
    answer = pow((x+SRGB_ALPHA)/(1+SRGB_ALPHA),2.4);
  return answer;
}

// =========================================================================
// utility functions 
inline double DistanceBetweenTwoPoints(const Vec3f &p1, const Vec3f &p2) {
  Vec3f v = p1-p2;
  return v.Length();
}

inline double AreaOfTriangle(double a, double b, double c) {
  // from the lengths of the 3 edges, compute the area
  // Area of Triangle = (using Heron's Formula)
  //  sqrt[s*(s-a)*(s-b)*(s-c)]
  //    where s = (a+b+c)/2
  // also... Area of Triangle = 0.5 * x * c
  double s = (a+b+c) / (double)2;
  return sqrt(s*(s-a)*(s-b)*(s-c));
}

inline double AreaOfTriangle(const Vec3f &a, const Vec3f &b, const Vec3f &c) {
  double aside = DistanceBetweenTwoPoints(a,b);
  double bside = DistanceBetweenTwoPoints(b,c);
  double cside = DistanceBetweenTwoPoints(c,a);
  return AreaOfTriangle(aside,bside,cside);
}

// utility function to generate random numbers used for sampling
inline Vec3f RandomUnitVector() {
  Vec3f tmp;
  while (1) {
    tmp = Vec3f(2*GLOBAL_mtrand.rand()-1,  // random real in [-1,1]
		2*GLOBAL_mtrand.rand()-1,  // random real in [-1,1]
		2*GLOBAL_mtrand.rand()-1); // random real in [-1,1]
    if (tmp.Length() < 1) break;
  }
  tmp.Normalize();
  return tmp;
}

// compute the perfect mirror direction
inline Vec3f MirrorDirection(const Vec3f &normal, const Vec3f &incoming) {
  double dot = incoming.Dot3(normal);
  Vec3f r = (incoming*-1.0f) + normal * (2 * dot);
  return r*-1.0f;
}

// compute a random diffuse direction
// (not the same as a uniform random direction on the hemisphere)
inline Vec3f RandomDiffuseDirection(const Vec3f &normal) {
  Vec3f answer = normal+RandomUnitVector();
  answer.Normalize();
  return answer;
}


#endif
