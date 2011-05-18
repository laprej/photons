#ifndef _RAY_TREE_H
#define _RAY_TREE_H

#include <vector>
#include "ray.h"

// ====================================================================
// ====================================================================
// data structure to store a segment

class Segment {

public:
  // CONSTRUCTOR
  Segment(const Ray &ray, double tstart, double tstop) {
    // first clamp the segment to "reasonable" values 
    // to make sure it is drawn correctly in OpenGL
    if (tstart < -1000) tstart = -1000;
    if (tstop  >  1000) tstop  =  1000;
    a = ray.pointAtParameter(tstart);
    b = ray.pointAtParameter(tstop); }

  // draw this ray on the screen
  void paint();
  
private:
  // REPRESENTATION
  Vec3f a;
  Vec3f b;
};

// ====================================================================
// ====================================================================
//
// This class only contains static variables and static member
// functions so there is no need to call the constructor, destructor
// etc.  It's just a wrapper for the ray tree visualization data.
//

class RayTree {

public:

  // most of the time the RayTree is NOT activated, so the segments are not updated
  static void Activate() { Clear(); activated = 1; }
  static void Deactivate() { activated = 0; }

  // when activated, these function calls store the segments of the tree
  static void AddMainSegment(const Ray &ray, double tstart, double tstop) {
    if (!activated) return;
    main_segments.push_back(Segment(ray,tstart,tstop));
  }
  static void AddShadowSegment(const Ray &ray, double tstart, double tstop) {
    if (!activated) return;
    shadow_segments.push_back(Segment(ray,tstart,tstop));
  }
  static void AddReflectedSegment(const Ray &ray, double tstart, double tstop) {
    if (!activated) return;
    reflected_segments.push_back(Segment(ray,tstart,tstop));
  }

  static void paint();

private:
  
  // HELPER FUNCTIONS
  static void paintHelper(const Vec4f &m,const Vec4f &s,const Vec4f &r,const Vec4f &t);
  static void Clear() {
    main_segments.clear();
    shadow_segments.clear();
    reflected_segments.clear();
  }
  
  // REPRESENTATION
  static int activated;
  static std::vector<Segment> main_segments;
  static std::vector<Segment> shadow_segments;
  static std::vector<Segment> reflected_segments;
};

// ====================================================================
// ====================================================================

#endif
