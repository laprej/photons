#include <cstdio>
#include <cstdlib>

// Included files for OpenGL Rendering
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "MersenneTwister.h"
#include "argparser.h"
#include "glCanvas.h"
#include "mesh.h"
#include "radiosity.h"
#include "photon_mapping.h"
#include "raytracer.h"
#include "utils.h"

// =========================================
// =========================================

MTRand GLOBAL_mtrand;

int main(int argc, char *argv[]) {
  
  // deterministic (repeatable) randomness
  // (comment out for "real" randomness)
  GLOBAL_mtrand = MTRand(37);

  ArgParser *args = new ArgParser(argc, argv);
  glutInit(&argc, argv);

  Mesh *mesh = new Mesh();
  mesh->Load(args->input_file,args);
  RayTracer *raytracer = new RayTracer(mesh,args);
  Radiosity *radiosity = new Radiosity(mesh,args);
  PhotonMapping *photon_mapping = new PhotonMapping(mesh,args);
  raytracer->setRadiosity(radiosity);
  raytracer->setPhotonMapping(photon_mapping);
  radiosity->setRayTracer(raytracer);
  radiosity->setPhotonMapping(photon_mapping);
  photon_mapping->setRayTracer(raytracer);
  photon_mapping->setRadiosity(radiosity);

  GLCanvas glcanvas;
  glcanvas.initialize(args,mesh,raytracer,radiosity,photon_mapping); 

  // well it never returns from the GLCanvas loop...
  delete args;
  return 0;
}

// =========================================
// =========================================
