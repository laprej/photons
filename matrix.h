#ifndef _MATRIX_H_
#define _MATRIX_H_

//
// originally implemented by Justin Legakis
//

#include <cmath>
#include <cassert>
#include "vectors.h"

// ====================================================================
// ====================================================================

class Matrix {

public:

  // -----------------------------------------------
  // CONSTRUCTORS, ASSIGNMENT OPERATOR, & DESTRUCTOR
  Matrix() { clear(); }
  Matrix(const Matrix& m);
  Matrix& operator=(const Matrix& m);
  
  // ----------------------------
  // SIMPLE ACCESSORS & MODIFIERS
  double get(int row, int col) const { 
    assert (row >= 0 && row < 4);
    assert (col >= 0 && col < 4);
    return data[row+col*4]; }
  void glGet(float float_data[16]) const {
    for (int i = 0; i < 16; i++) {
      float_data[i] = (float)data[i];
    }
  }
  void set(int row, int col, double v) {
    assert (row >= 0 && row < 4);
    assert (col >= 0 && col < 4);
    data[row+col*4] = v; }
  void setToIdentity(); // 1s on the diagonal
  void clear(); // 0s everywhere

  // --------------------------
  // STANDARD MATRIX OPERATIONS
  void Transpose(Matrix &m) const;
  void Transpose() { Transpose(*this); }
  int Inverse(Matrix &m, double epsilon = 1e-08) const;
  int Inverse(double epsilon = 1e-08) { return Inverse(*this,epsilon); }

  // --------------------
  // OVERLOADED OPERATORS
  friend Matrix operator+(const Matrix &m1, const Matrix &m2);
  friend Matrix operator-(const Matrix &m1, const Matrix &m2);
  friend Matrix operator*(const Matrix &m1, const Matrix &m2);
  friend Vec3f operator*(const Matrix &m1, const Vec3f &v);
  friend Matrix operator*(const Matrix &m1, double d);
  friend Matrix operator*(double d, const Matrix &m) { return m * d; }
  Matrix& operator+=(const Matrix& m) { *this = *this + m; return *this; }
  Matrix& operator-=(const Matrix& m) { *this = *this - m; return *this; }
  Matrix& operator*=(const double d)  { *this = *this * d; return *this; }
  Matrix& operator*=(const Matrix& m) { *this = *this * m; return *this; }

  // ---------------
  // TRANSFORMATIONS
  static Matrix MakeTranslation(const Vec3f &v);
  static Matrix MakeScale(const Vec3f &v);
  static Matrix MakeScale(double s) { return MakeScale(Vec3f(s,s,s)); }
  static Matrix MakeXRotation(double theta);
  static Matrix MakeYRotation(double theta);
  static Matrix MakeZRotation(double theta);
  static Matrix MakeAxisRotation(const Vec3f &v, double theta);
 
  // Use to transform a point with a matrix
  // that may include translation
  void Transform(Vec4f &v) const;
  void Transform(Vec3f &v) const {
    Vec4f v2 = Vec4f(v.x(),v.y(),v.z(),1);
    Transform(v2);
    v.set(v2.x()/v2.w(),v2.y()/v2.w(),v2.z()/v2.w()); }

  // Use to transform the direction of the ray
  // (ignores any translation)
  void TransformDirection(Vec3f &v) const {
    Vec4f v2 = Vec4f(v.x(),v.y(),v.z(),0);
    Transform(v2);
    v.set(v2.x(),v2.y(),v2.z()); }

  // --------------
  // INPUT / OUTPUT
  friend std::ostream& operator<<(std::ostream &ostr, const Matrix &m);
  friend std::istream& operator>>(std::istream &istr, Matrix &m);

  // ------------
  // DETERMINANTS
  static double det4x4(double a1, double a2, double a3, double a4, 
		      double b1, double b2, double b3, double b4, 
		      double c1, double c2, double c3, double c4, 
		      double d1, double d2, double d3, double d4);
  static double det3x3(double a1,double a2,double a3,
		      double b1,double b2,double b3,
		      double c1,double c2,double c3);
  static double det2x2(double a, double b,
		      double c, double d);
  
private:
  // --------------
  // REPRESENTATION 
  // column-major order
  double data[16];
};

// ====================================================================
// ====================================================================

#endif
