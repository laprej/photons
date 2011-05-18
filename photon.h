#ifndef _PHOTON_H_
#define _PHOTON_H_

#include <iostream>
#include "vectors.h"

// ===========================================================
// Class to store the information when a photon hits a surface

class Photon {
 public:

  // CONSTRUCTOR
  Photon(const Vec3f &p, const Vec3f &d, const Vec3f &e, int b) :
    position(p),direction_from(d),energy(e),bounce(b) {}

  // ACCESSORS
  const Vec3f& getPosition() const { return position; }
  const Vec3f& getDirectionFrom() const { return direction_from; }
  const Vec3f& getEnergy() const { return energy; }
  int whichBounce() const { return bounce; }

 private:
  // REPRESENTATION
  Vec3f position;
  Vec3f direction_from;
  Vec3f energy;
  int bounce;
};

#endif
