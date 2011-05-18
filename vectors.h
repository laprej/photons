#ifndef _VECTORS_H_
#define _VECTORS_H_

//
// originally implemented by Justin Legakis
//

#include <iostream>
#include <cmath>
#include <cassert>

class Matrix;

// ====================================================================
// ====================================================================

class Vec3f {

public:

  // -----------------------------------------------
  // CONSTRUCTORS, ASSIGNMENT OPERATOR, & DESTRUCTOR
  Vec3f() { data[0] = data[1] = data[2] = 0; }
  Vec3f(const Vec3f &V) {
    data[0] = V.data[0];
    data[1] = V.data[1];
    data[2] = V.data[2]; }
  Vec3f(double d0, double d1, double d2) {
    data[0] = d0;
    data[1] = d1;
    data[2] = d2; }
  const Vec3f& operator=(const Vec3f &V) {
    data[0] = V.data[0];
    data[1] = V.data[1];
    data[2] = V.data[2];
    return *this; }

  // ----------------------------
  // SIMPLE ACCESSORS & MODIFIERS
  double operator[](int i) const { 
    assert (i >= 0 && i < 3); 
    return data[i]; }
  double x() const { return data[0]; }
  double y() const { return data[1]; }
  double z() const { return data[2]; }
  double r() const { return data[0]; }
  double g() const { return data[1]; }
  double b() const { return data[2]; }
  void setx(double x) { data[0]=x; }
  void sety(double y) { data[1]=y; }
  void setz(double z) { data[2]=z; }
  void set(double d0, double d1, double d2) {
    data[0] = d0;
    data[1] = d1;
    data[2] = d2; }

  // ----------------
  // EQUALITY TESTING 
  int operator==(const Vec3f &V) {
    return ((data[0] == V.data[0]) &&
	    (data[1] == V.data[1]) &&
	    (data[2] == V.data[2])); }
  int operator!=(const Vec3f &V) {
    return ((data[0] != V.data[0]) ||
	    (data[1] != V.data[1]) ||
	    (data[2] != V.data[2])); }

  // ------------------------
  // COMMON VECTOR OPERATIONS
  double Length() const {
    return sqrt(data[0]*data[0]+data[1]*data[1]+data[2]*data[2]); }
  void Normalize() {
    double length = Length();
    if (length > 0) { Scale (1/length); } }
  void Scale(double d) { Scale(d,d,d); }
  void Scale(double d0, double d1, double d2) {
    data[0] *= d0;
    data[1] *= d1;
    data[2] *= d2; }
  void Negate() { Scale(-1.0); }
  double Dot3(const Vec3f &V) const {
    return data[0] * V.data[0] +
      data[1] * V.data[1] +
      data[2] * V.data[2] ; }
  static void Cross3(Vec3f &c, const Vec3f &v1, const Vec3f &v2) {
    double x = v1.data[1]*v2.data[2] - v1.data[2]*v2.data[1];
    double y = v1.data[2]*v2.data[0] - v1.data[0]*v2.data[2];
    double z = v1.data[0]*v2.data[1] - v1.data[1]*v2.data[0];
    c.data[0] = x; c.data[1] = y; c.data[2] = z; }

  // ---------------------
  // VECTOR MATH OPERATORS
  Vec3f& operator+=(const Vec3f &V) {
    data[0] += V.data[0];
    data[1] += V.data[1];
    data[2] += V.data[2];
    return *this; }
  Vec3f& operator-=(const Vec3f &V) {
    data[0] -= V.data[0];
    data[1] -= V.data[1];
    data[2] -= V.data[2];
    return *this; }
  Vec3f& operator*=(double d) {
    data[0] *= d;
    data[1] *= d;
    data[2] *= d;
    return *this; }
  Vec3f& operator/=(double d) {
    data[0] /= d;
    data[1] /= d;
    data[2] /= d;
    return *this; }  
  friend Vec3f operator+(const Vec3f &v1, const Vec3f &v2) { 
    Vec3f v3 = v1; v3 += v2; return v3; }
  friend Vec3f operator-(const Vec3f &v1) {
    Vec3f v2 = v1; v2.Negate(); return v2; }
  friend Vec3f operator-(const Vec3f &v1, const Vec3f &v2) {
    Vec3f v3 = v1; v3 -= v2; return v3; }
  friend Vec3f operator*(const Vec3f &v1, double d) {
    Vec3f v2 = v1; v2.Scale(d); return v2; }
  friend Vec3f operator*(const Vec3f &v1, const Vec3f &v2) {
    Vec3f v3 = v1; v3.Scale(v2.x(),v2.y(),v2.z()); return v3; }
  friend Vec3f operator*(double d, const Vec3f &v1) {
    return v1 * d; }

  // --------------
  // INPUT / OUTPUT
  friend std::ostream& operator<< (std::ostream &ostr, const Vec3f &v) {
    ostr << v.data[0] << " " << v.data[1] << " " << v.data[2] << std::endl; 
    return ostr; }
  friend std::istream& operator>> (std::istream &istr, Vec3f &v) {
    istr >> v.data[0] >> v.data[1] >> v.data[2];
    return istr; }
  
private:

  friend class Matrix;

  // REPRESENTATION
  double		data[3];
  
};

// ====================================================================
// ====================================================================

class Vec4f {
  
public:

  // CONSTRUCTORS, ASSIGNMENT OPERATOR, & DESTRUCTOR
  Vec4f() { data[0] = data[1] = data[2] = data[3] = 0; }
  Vec4f(const Vec4f &V) {
    data[0] = V.data[0];
    data[1] = V.data[1];
    data[2] = V.data[2];
    data[3] = V.data[3]; }
  Vec4f(double d0, double d1, double d2, double d3) {
    data[0] = d0;
    data[1] = d1;
    data[2] = d2;
    data[3] = d3; }
  Vec4f& operator=(const Vec4f &V) {
    data[0] = V.data[0];
    data[1] = V.data[1];
    data[2] = V.data[2];
    data[3] = V.data[3];
    return *this; }

  // SIMPLE ACCESSORS & MODIFIERS
  double operator[](int i) const { 
    assert (i >= 0 && i < 4); 
    return data[i]; }
  double x() const { return data[0]; }
  double y() const { return data[1]; }
  double z() const { return data[2]; }
  double w() const { return data[3]; }
  double r() const { return data[0]; }
  double g() const { return data[1]; }
  double b() const { return data[2]; }
  double a() const { return data[3]; }
  void set(double d0, double d1, double d2, double d3) {
    data[0] = d0;
    data[1] = d1;
    data[2] = d2;
    data[3] = d3; }

  // ----------------
  // EQUALITY TESTING
  int operator==(const Vec4f &V) const {
    return ((data[0] == V.data[0]) &&
	    (data[1] == V.data[1]) &&
	    (data[2] == V.data[2]) &&
	    (data[3] == V.data[3])); }
  int operator!=(const Vec4f &V) const {
    return ((data[0] != V.data[0]) ||
	    (data[1] != V.data[1]) ||
	    (data[2] != V.data[2]) ||
	    (data[3] != V.data[3])); }

  // ------------------------
  // COMMON VECTOR OPERATIONS
  double Length() const {
    return (double)sqrt(data[0]*data[0]+data[1]*data[1]+data[2]*data[2]+data[3]*data[3]); }
  void Normalize() {
    double l = Length();
    if (l > 0) {
      data[0] /= l;
      data[1] /= l;
      data[2] /= l; }}
  void Scale(double d) { Scale(d,d,d,d); }
  void Scale(double d0, double d1, double d2, double d3) {
    data[0] *= d0;
    data[1] *= d1;
    data[2] *= d2;
    data[3] *= d3; }
  void Negate() { Scale(-1.0); }
  double Dot4(const Vec4f &V) const {
    return data[0] * V.data[0] +
      data[1] * V.data[1] +
      data[2] * V.data[2] +
      data[3] * V.data[3]; }
  static void Cross3(Vec4f &c, const Vec4f &v1, const Vec4f &v2) {
    double x = v1.data[1]*v2.data[2] - v1.data[2]*v2.data[1];
    double y = v1.data[2]*v2.data[0] - v1.data[0]*v2.data[2];
    double z = v1.data[0]*v2.data[1] - v1.data[1]*v2.data[0];
    c.data[0] = x; c.data[1] = y; c.data[2] = z; c.data[3] = 1; }
  void DivideByW() {
    if (data[3] != 0) {
      data[0] /= data[3];
      data[1] /= data[3];
      data[2] /= data[3];
    } else {
      data[0] = data[1] = data[2] = 0; }
    data[3] = 1; }
  
  // ---------------------
  // VECTOR MATH OPERATORS
  Vec4f& operator+=(const Vec4f &V) {
    data[0] += V.data[0];
    data[1] += V.data[1];
    data[2] += V.data[2];
    data[3] += V.data[3];
    return *this; }
  Vec4f& operator-=(const Vec4f &V) {
    data[0] -= V.data[0];
    data[1] -= V.data[1];
    data[2] -= V.data[2];
    data[3] -= V.data[3];
    return *this; }
  Vec4f& operator*=(double d) {
    data[0] *= d;
    data[1] *= d;
    data[2] *= d;
    data[3] *= d;
    return *this; }
  Vec4f& operator/=(double d) {
    data[0] /= d;
    data[1] /= d;
    data[2] /= d;
    data[3] /= d;
    return *this; }

  // --------------
  // INPUT / OUTPUT
  friend std::ostream& operator<< (std::ostream &ostr, const Vec4f &v) {
    ostr << v.data[0] << " " << v.data[1] << " " << v.data[2] << " " << v.data[3] << std::endl; 
    return ostr; }
  friend std::istream& operator>> (std::istream &istr, Vec4f &v) {
    istr >> v.data[0] >> v.data[1] >> v.data[2] >> v.data[3];
    return istr; }

private:

  friend class Matrix;

  // REPRESENTATION
  double		data[4];
  
};

// ====================================================================
// ====================================================================

#endif

