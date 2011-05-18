#ifndef _BOUNDING_BOX_H_
#define _BOUNDING_BOX_H_

#include <cassert>
#include "vectors.h"
#include "utils.h"

// ====================================================================
// ====================================================================

class BoundingBox {

public:

  // CONSTRUCTOR & DESTRUCTOR
  BoundingBox() { }
  BoundingBox(const Vec3f &v) { Set(v,v); }
  BoundingBox(const Vec3f &_min, const Vec3f &_max) {
    Set(_min,_max); }
  ~BoundingBox() {}

  // ACCESSORS
  void Get(Vec3f &_min, Vec3f &_max) const {
    _min = min;
    _max = max; }
  const Vec3f& getMin() const { return min; }
  const Vec3f& getMax() const { return max; }

  Vec3f getCenter() const {
    return min + 0.5*(max-min);
  }

  double maxDim() const {
    double x = max.x() - min.x();
    double y = max.y() - min.y();
    double z = max.z() - min.z();
    return std::max(x,std::max(y,z));
  }

  // MODIFIERS
  void Set(BoundingBox *bb) {
    assert (bb != NULL);
    min = bb->min;
    max = bb->max; }
  void Set(Vec3f _min, Vec3f _max) {
    assert (min.x() <= max.x() &&
	    min.y() <= max.y() &&
	    min.z() <= max.z());
    min = _min;
    max = _max; }
  void Extend(const Vec3f &v) {
    min = Vec3f(std::min(min.x(),v.x()),
		std::min(min.y(),v.y()),
		std::min(min.z(),v.z()));
    max = Vec3f(std::max(max.x(),v.x()),
		std::max(max.y(),v.y()),
		std::max(max.z(),v.z())); }
  void Extend(const BoundingBox &bb) {
    Extend(bb.min);
    Extend(bb.max); }

  friend std::ostream& operator<<(std::ostream &ostr, const BoundingBox &bbox) {
    ostr << "min " << bbox.min;
    ostr << "max " << bbox.max;
    return ostr;
  }

private:

  // REPRESENTATION
  Vec3f min;
  Vec3f max;
};

// ====================================================================
// ====================================================================

#endif
