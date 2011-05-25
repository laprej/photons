#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#include <cstdio>
#include <cstdlib>

// Included files for OpenGL Rendering
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "vectors.h"
#include "image.h"

class ArgParser;
class Ray;
class Hit;

// ====================================================================
// ====================================================================
// A simple Phong-like material 

class Material {
    
public:
    
    Material(const std::string &texture_file,const Vec3f &d_color,const Vec3f &r_color,const Vec3f &e_color, const Vec3f &t_color) {
        textureFile = texture_file;
        if (textureFile != "") {
            image = new Image(textureFile);
            ComputeAverageTextureColor();
        } else {
            diffuseColor = d_color;
            image = NULL;
        }
        reflectiveColor = r_color;
        emittedColor = e_color;
        transmittedColor = t_color;
        // need to initialize texture_id after glut has started
        texture_id = 0;
    }
    
    ~Material();
    
    // ACCESSORS
    const Vec3f& getDiffuseColor() const { return diffuseColor; }
    const Vec3f getDiffuseColor(double s, double t) const;
    const Vec3f& getReflectiveColor() const { return reflectiveColor; }
    const Vec3f& getEmittedColor() const { return emittedColor; }
    const Vec3f& getTransmissiveColor() const { return transmittedColor; }
    bool hasTextureMap() const { return (textureFile != ""); } 
    GLuint getTextureID();
    
    // SHADE: compute the contribution to local illumination at this
    // point for a particular light source
    Vec3f Shade
    (const Ray &ray, const Hit &hit, const Vec3f &dirToLight, 
     const Vec3f &lightColor, ArgParser *args) const;
    
protected:
    
    Material() { exit(0); }
    Material(const Material& m) { exit(0); }
    const Material& operator=(const Material& m) { exit(0); }
    
    // helper function
    void ComputeAverageTextureColor();
    
    // REPRESENTATION
    Vec3f diffuseColor;
    Vec3f reflectiveColor;
    Vec3f emittedColor;
    Vec3f transmittedColor;
    
    std::string textureFile;
    GLuint texture_id;
    Image *image;
};

// ====================================================================
// ====================================================================

#endif

