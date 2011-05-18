// ====================================================================
// GLCanvas class by Rob Jagnow.
// ====================================================================

#ifndef _GL_CANVAS_H_
#define _GL_CANVAS_H_

#include <cstdlib>
#include <cassert>
#include "vectors.h"

class ArgParser;
class Mesh;
class RayTracer;
class Radiosity;
class PhotonMapping;

// ====================================================================
// NOTE:  All the methods and variables of this class are static
// ====================================================================

class GLCanvas {

public:

  // Set up the canvas and enter the rendering loop
  // Note that this function will not return but can be
  // terminated by calling 'exit(0)'
  static void initialize(ArgParser *_args, Mesh *_mesh, 
			 RayTracer *_raytracer, Radiosity *_radiosity, PhotonMapping *_photon_mapping);
  static void Render();

private:

  static void InitLight();

  // various static variables
  static ArgParser *args;
  static Mesh *mesh;
  static RayTracer *raytracer;
  static Radiosity *radiosity;
  static PhotonMapping *photon_mapping;

  static int display_list_index;

  // state of the mouse cursor
  static int mouseButton;
  static int mouseX;
  static int mouseY;
  static bool shiftPressed;
  static bool controlPressed;
  static bool altPressed;
  static int raytracing_x;
  static int raytracing_y;
  static int raytracing_skip;

  // Callback functions for mouse and keyboard events
  static void display(void);
  static void reshape(int w, int h);
  static void mouse(int button, int state, int x, int y);
  static void motion(int x, int y);
  static void keyboard(unsigned char key, int x, int y);
  static void idle();
  
  static int DrawPixel();
  static Vec3f TraceRay(int i, int j);
};

// ====================================================================

int HandleGLError();

// ====================================================================

#endif
