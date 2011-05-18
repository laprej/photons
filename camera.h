#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <cassert>
#include <iostream>
#include <fstream>
#include "vectors.h"
#include "ray.h"

// ====================================================================

class Camera {

public:
  // CONSTRUCTOR & DESTRUCTOR
  Camera(const Vec3f &c, const Vec3f &poi, const Vec3f &u);
  virtual ~Camera() {}

  // RENDERING
  virtual Ray generateRay(double x, double y) = 0;

  // GL NAVIGATION
  virtual void glInit(int w, int h) = 0;
  void glPlaceCamera(void);
  void dollyCamera(double dist);
  virtual void zoomCamera(double dist) = 0;
  void truckCamera(double dx, double dy);
  void rotateCamera(double rx, double ry);
  friend std::ostream& operator<<(std::ostream &ostr, const Camera &c);

protected:
  Camera() { assert(0); } // don't use

  // HELPER FUNCTIONS
  Vec3f getHorizontal() const {
    Vec3f answer;
    Vec3f::Cross3(answer, getDirection(), up);
    answer.Normalize();
    return answer; }
  Vec3f getScreenUp() const {
    Vec3f answer;
    Vec3f::Cross3(answer, getHorizontal(), getDirection());
    return answer; }
  Vec3f getDirection() const {
    Vec3f answer = point_of_interest - camera_position;
    answer.Normalize();
    return answer; }

  // REPRESENTATION
  Vec3f point_of_interest;
  Vec3f camera_position;
  Vec3f up;
  int width;
  int height;
};

// ====================================================================

class OrthographicCamera : public Camera {

public:
  // CONSTRUCTOR & DESTRUCTOR
  OrthographicCamera(const Vec3f &c = Vec3f(0,0,1), 
		     const Vec3f &poi = Vec3f(0,0,0), 
		     const Vec3f &u = Vec3f(0,1,0),
		     double s=100);  

  // RENDERING
  Ray generateRay(double x, double y);

  // GL NAVIGATION
  void glInit(int w, int h);
  void zoomCamera(double factor);
  friend std::ostream& operator<<(std::ostream &ostr, const OrthographicCamera &c);
  friend std::istream& operator>>(std::istream &istr, OrthographicCamera &c);

private:

  // REPRESENTATION
  double size;
};

// ====================================================================

class PerspectiveCamera : public Camera {

public:
  // CONSTRUCTOR & DESTRUCTOR
  PerspectiveCamera(const Vec3f &c = Vec3f(0,0,1), 
		    const Vec3f &poi = Vec3f(0,0,0), 
		    const Vec3f &u = Vec3f(0,1,0),
		    double a = 45);

  // RENDERING
  Ray generateRay(double x, double y);

  // GL NAVIGATION
  void glInit(int w, int h);
  void zoomCamera(double dist);
  friend std::ostream& operator<<(std::ostream &ostr, const PerspectiveCamera &c);
  friend std::istream& operator>>(std::istream &istr, PerspectiveCamera &c);
  
private:

  // REPRESENTATION
  double angle;
};

// ====================================================================

#endif

