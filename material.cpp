#include "material.h"
#include "utils.h"
#include "ray.h"
#include "hit.h"

// ==================================================================
// DESTRUCTOR
// ==================================================================
Material::~Material() {
  if (hasTextureMap()) {
    glDeleteTextures(1,&texture_id);
    assert (image != NULL);
    delete image;
  }
}

// ==================================================================
// TEXTURE LOOKUP FOR DIFFUSE COLOR
// ==================================================================
const Vec3f Material::getDiffuseColor(double s, double t) const {
  if (!hasTextureMap()) return diffuseColor; 

  assert (image != NULL);

  // simple biliear interpolation on the closest 4 pixels in the texture
  double i_ = (s * image->Width());
  double j_ = (t * image->Height());
  int i1 = int(s * image->Width());
  int j1 = int(t * image->Height());
  double dx = i_-i1;
  double dy = j_-j1;
  i1 %= image->Width();
  j1 %= image->Height();
  if (i1 < 0) i1 += image->Width();
  if (j1 < 0) j1 += image->Height();
  assert (i1 >= 0 && i1 < image->Width());
  assert (j1 >= 0 && j1 < image->Height());
  int i2 = i1+1;  if (i2 == image->Width()) i2-=1;
  int j2 = j1+1;  if (j2 == image->Height()) j2-=1;
  assert (i2 >= 0 && i2 < image->Width());
  assert (j2 >= 0 && j2 < image->Height());

  // average the 4 colors
  Color c1 = image->GetPixel(i1,j1);
  Color c2 = image->GetPixel(i1,j2);
  Color c3 = image->GetPixel(i2,j1);
  Color c4 = image->GetPixel(i2,j2);

  // we assume the texture is stored in sRGB and convert to linear for
  // computation.  It will be converted back to sRGB before display.
  Vec3f c1v(srgb_to_linear(c1.r/255.0),srgb_to_linear(c1.g/255.0),srgb_to_linear(c1.b/255.0));
  Vec3f c2v(srgb_to_linear(c2.r/255.0),srgb_to_linear(c2.g/255.0),srgb_to_linear(c2.b/255.0));
  Vec3f c3v(srgb_to_linear(c3.r/255.0),srgb_to_linear(c3.g/255.0),srgb_to_linear(c3.b/255.0));
  Vec3f c4v(srgb_to_linear(c4.r/255.0),srgb_to_linear(c4.g/255.0),srgb_to_linear(c4.b/255.0));

  return c1v*(1-dx)*(1-dy) + c2v*(1-dx)*dy + c3v*dx*(1-dy) + c4v*dx*dy;
}

// ==================================================================
// OpenGL setup for textures
// ==================================================================
GLuint Material::getTextureID() { 
  assert (hasTextureMap()); 

  // if this is the first time the texture is being used, we must
  // initialize it
  if (texture_id == 0)  {
    glGenTextures(1,&texture_id);
    assert (texture_id != 0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    // select modulate to mix texture with color for shading
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    // when texture area is small, bilinear filter the closest mipmap
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		     GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the original
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    // the texture wraps over at the edges (repeat)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    // to be most compatible, textures should be square and a power of 2
    assert (image->Width() == image->Height());
    assert (image->Width() == 256);
    // build our texture mipmaps
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, image->Width(), image->Height(),
		       GL_RGB, GL_UNSIGNED_BYTE, image->getGLPixelData());
  }
  
  return texture_id;
}

// ==================================================================
// An average texture color, a hack for use in radiosity
// ==================================================================
void Material::ComputeAverageTextureColor() {
  assert (hasTextureMap());
  double r = 0;
  double g = 0;
  double b = 0;
  for (int i = 0; i < image->Width(); i++) {
    for (int j = 0; j < image->Height(); j++) {
      Color c = image->GetPixel(i,j);
       r += srgb_to_linear(c.r/255.0);
       g += srgb_to_linear(c.g/255.0);
       b += srgb_to_linear(c.b/255.0);
    }
  }
  int count = image->Width() * image->Height();
  r /= double(count);
  g /= double(count);
  b /= double(count);
  diffuseColor = Vec3f(r,g,b);
}

// ==================================================================
// PHONG LOCAL ILLUMINATION

// this function should be called to compute the light contributed by
// a particular light source to the intersection point.  Note that
// this function does not calculate any global effects (e.g., shadows). 

Vec3f Material::Shade(const Ray &ray, const Hit &hit, 
                      const Vec3f &dirToLight, 
                      const Vec3f &lightColor, ArgParser *args) const {
  
  Vec3f point = ray.pointAtParameter(hit.getT());
  Vec3f n = hit.getNormal();
  Vec3f e = ray.getDirection()*-1.0f;
  Vec3f l = dirToLight;
  
  Vec3f answer = Vec3f(0,0,0);

  // emitted component
  // -----------------
  answer += getEmittedColor();

  // diffuse component
  // -----------------
  double dot_nl = n.Dot3(l);
  if (dot_nl < 0) dot_nl = 0;
  answer += lightColor * getDiffuseColor(hit.get_s(),hit.get_t()) * dot_nl;

  // specular component (Phong)
  // ------------------
  // make up reasonable values for other Phong parameters
  Vec3f specularColor = reflectiveColor;
  double exponent = 100;

  // compute ideal reflection angle
  Vec3f r = (l*-1.0f) + n * (2 * dot_nl);
  r.Normalize();
  double dot_er = e.Dot3(r);
  if (dot_er < 0) dot_er = 0;
  answer += lightColor*specularColor*pow(dot_er,exponent)* dot_nl;

  return answer;
}

// ==================================================================
