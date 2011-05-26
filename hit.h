#ifndef _HIT_H_
#define _HIT_H_

#include <float.h>
#include <ostream>
#include "vectors.h"
#include "ray.h"

class Material;

// Hit class mostly copied from Peter Shirley and Keith Morley
// ====================================================================
// ====================================================================

class Hit {
    
public:
    
    // CONSTRUCTOR & DESTRUCTOR
    Hit(double init_t = FLT_MAX) { 
        t = init_t;
        material = NULL;
        normal = Vec3f(0,0,0); 
        texture_s = 0;
        texture_t = 0;
    }
    Hit(const Hit &h) { 
        t = h.t; 
        material = h.material; 
        normal = h.normal; 
        texture_s = h.texture_s;
        texture_t = h.texture_t;
    }
    ~Hit() {}
    
    // ACCESSORS
    double getT() const { return t; }
    Material* getMaterial() const { return material; }
    Vec3f getNormal() const { return normal; }
    double get_s() const { return texture_s; }
    double get_t() const { return texture_t; }
    
    // MODIFIER
    void set(double _t, Material *m, Vec3f n) {
        t = _t; material = m; normal = n; 
        texture_s = 0; texture_t = 0; }
    
    void setTextureCoords(double t_s, double t_t) {
        texture_s = t_s; texture_t = t_t; 
    }
    
private: 
    
    // REPRESENTATION
    double t;
    Material *material;
    Vec3f normal;
    double texture_s, texture_t;
};

inline std::ostream &operator<<(std::ostream &os, const Hit &h) {
    os << "Hit <" <<h.getT()<<", "<<h.getNormal()<<">";
    return os;
}
// ====================================================================
// ====================================================================

#endif
