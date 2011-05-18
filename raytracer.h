#ifndef _RAY_TRACER_
#define _RAY_TRACER_

#include <cassert>
#include <vector>
#include "ray.h"
#include "hit.h"

class Mesh;
class ArgParser;
class Radiosity;
class PhotonMapping;

// ====================================================================
// ====================================================================

class RayTracer {

public:

  // CONSTRUCTOR & DESTRUCTOR
  RayTracer(Mesh *m, ArgParser *a) {
    mesh = m;
    args = a;
  }  
  // set access to the other modules for hybrid rendering options
  void setRadiosity(Radiosity *r) { radiosity = r; }
  void setPhotonMapping(PhotonMapping *pm) { photon_mapping = pm; }
  
  // casts a single ray through the scene geometry and finds the closest hit
  bool CastRay(Ray &ray, Hit &h, bool use_sphere_patches) const;

  // does the recursive work
  Vec3f TraceRay(Ray &ray, Hit &hit, int bounce_count = 0) const;

private:

  // REPRESENTATION
  Mesh *mesh;
  ArgParser *args;
  Radiosity *radiosity;
  PhotonMapping *photon_mapping;
};

// ====================================================================
// ====================================================================

#endif
