#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "primitive.h"
#include "vectors.h"

// ====================================================================
// ====================================================================
// Simple implicit repesentation of a sphere, that can also be
// rasterized for use in radiosity.

class Sphere : public Primitive {
    
public:
    // CONSTRUCTOR & DESTRUCTOR
    Sphere(const Vec3f &c, double r, Material *m) {
        center = c; radius = r; material = m;
        assert (radius >= 0); }
    
    // for ray tracing
    virtual bool intersect(const Ray &r, Hit &h) const;
    
    // for OpenGL rendering & radiosity
    void addRasterizedFaces(Mesh *m, ArgParser *args);
    
    friend std::ostream& operator<< (std::ostream &ostr, const Sphere &s) {
        ostr << "sphere centered at (" << s.center.x() << ", " << s.center.y() << ", " << s.center.z() << "), radius " << s.radius;
        return ostr; }
private:
    
    // REPRESENTATION
    Vec3f center;
    double radius;
};

// ====================================================================
// ====================================================================

#endif
