#ifndef __ARG_PARSER_H__
#define __ARG_PARSER_H__

#include <cstring>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include "vectors.h"
#include "MersenneTwister.h"

// VISUALIZATION MODES FOR RADIOSITY
#define NUM_RENDER_MODES 6
enum RENDER_MODE { RENDER_MATERIALS, RENDER_RADIANCE, RENDER_FORM_FACTORS, 
		   RENDER_LIGHTS, RENDER_UNDISTRIBUTED, RENDER_ABSORBED };


// ======================================================================
// Class to collect all the high-level rendering parameters controlled
// by the command line or the keyboard input
// ======================================================================

class ArgParser {

public:

  ArgParser() { DefaultValues(); }

  ArgParser(int argc, char *argv[]) {
    DefaultValues();

    for (int i = 1; i < argc; i++) {
      if (!strcmp(argv[i],"-input") || !strcmp(argv[i],"-i")) {
	i++; assert (i < argc); 
	input_file = argv[i];
      } else if (!strcmp(argv[i],"-size")) {
	i++; assert (i < argc); 
	width = atoi(argv[i]);
	i++; assert (i < argc); 
	height = atoi(argv[i]);
      } else if (!strcmp(argv[i],"-num_form_factor_samples")) {
	i++; assert (i < argc); 
	num_form_factor_samples = atoi(argv[i]);
      } else if (!strcmp(argv[i],"-sphere_rasterization")) {
	i++; assert (i < argc); 
	sphere_horiz = atoi(argv[i]);
	if (sphere_horiz % 2 == 1) sphere_horiz++; 
	i++; assert (i < argc); 
	sphere_vert = atoi(argv[i]);
      } else if (!strcmp(argv[i],"-cylinder_ring_rasterization")) {
	i++; assert (i < argc); 
	cylinder_ring_rasterization = atoi(argv[i]);

      } else if (!strcmp(argv[i],"-num_bounces")) {
	i++; assert (i < argc); 
	num_bounces = atoi(argv[i]);
      } else if (!strcmp(argv[i],"-num_shadow_samples")) {
	i++; assert (i < argc); 
	num_shadow_samples = atoi(argv[i]);
      } else if (!strcmp(argv[i],"-num_antialias_samples")) {
	i++; assert (i < argc); 
	num_antialias_samples = atoi(argv[i]);
	assert (num_antialias_samples > 0);
      } else if (!strcmp(argv[i],"-ambient_light")) {
	i++; assert (i < argc);
	double r = atof(argv[i]);
	i++; assert (i < argc);
	double g = atof(argv[i]);
	i++; assert (i < argc);
	double b = atof(argv[i]);
	ambient_light = Vec3f(r,g,b);
      } else if (!strcmp(argv[i],"-num_photons_to_shoot")) {
	i++; assert (i < argc);
	num_photons_to_shoot = atoi(argv[i]);
      } else if (!strcmp(argv[i],"-num_photons_to_collect")) {
	i++; assert (i < argc);
	num_photons_to_collect = atoi(argv[i]);
      } else if (!strcmp(argv[i],"-gather_indirect")) {
	gather_indirect = true;
      } else {
	printf ("whoops error with command line argument %d: '%s'\n",i,argv[i]);
	assert(0);
      }
    }
  }

  void DefaultValues() {
    // BASIC RENDERING PARAMETERS
    input_file = NULL;
    width = 400;
    height = 400;
    raytracing_animation = false;
    radiosity_animation = false;

    // RADIOSITY PARAMETERS
    render_mode = RENDER_MATERIALS;
    interpolate = false;
    wireframe = false;
    num_form_factor_samples = 1;
    sphere_horiz = 8;
    sphere_vert = 6;
    cylinder_ring_rasterization = 20; 

    // RAYTRACING PARAMETERS
    num_bounces = 0;
    num_shadow_samples = 0;
    num_antialias_samples = 1;
    ambient_light = Vec3f(0.1,0.1,0.1);
    intersect_backfacing = false;

    // PHOTON MAPPING PARAMETERS
    render_photons = true;
    render_kdtree = true;
    num_photons_to_shoot = 10000;
    num_photons_to_collect = 100;
    gather_indirect = false;
  }

  // ==============
  // REPRESENTATION
  // all public! (no accessors)

  // BASIC RENDERING PARAMETERS
  char *input_file;
  int width;
  int height;
  bool raytracing_animation;
  bool radiosity_animation;

  // RADIOSITY PARAMETERS
  enum RENDER_MODE render_mode;
  bool interpolate;
  bool wireframe;
  int num_form_factor_samples;
  int sphere_horiz;
  int sphere_vert;
  int cylinder_ring_rasterization;

  // RAYTRACING PARAMETERS
  int num_bounces;
  int num_shadow_samples;
  int num_antialias_samples;
  Vec3f ambient_light;
  bool intersect_backfacing;

  // PHOTON MAPPING PARAMETERS
  int num_photons_to_shoot;
  int num_photons_to_collect;
  bool render_photons;
  bool render_kdtree;
  bool gather_indirect;
};

#endif
