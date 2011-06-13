#ifndef _HASH_H_
#define _HASH_H_

// to handle different platforms with different variants of a developing standard
// NOTE: You may need to adjust these depending on your installation
#ifdef __APPLE__
#include <ext/hash_map>
#else
#ifdef __CYGWIN__
#include <ext/hash_map>
#else
// UNIX
#ifdef __FREEBSD__
// for older versions of unix without unordered maps (e.g., the submission server)
#include <ext/hash_map>
#else
// newer versions 
#include <unordered_map>
#endif
#endif
#endif

class Edge;
class Triangle;
#include "vertex.h"

#define LARGE_PRIME_A 10007
#define LARGE_PRIME_B 11003


// ===================================================================================
// DIRECTED EDGES are stored in a hash table using a simple hash
// function based on the indices of the start and end vertices
// ===================================================================================

inline size_t ordered_two_int_hash(size_t a, size_t b) {
  return LARGE_PRIME_A * a + LARGE_PRIME_B * b;
}

struct orderedvertexpairhash {
  size_t operator()(std::pair<Vertex*,Vertex*> p) const {
    return ordered_two_int_hash(p.first->getIndex(),p.second->getIndex());
  }
};

struct orderedsamevertexpair {
  bool operator()(std::pair<Vertex*,Vertex*> p1, std::pair<Vertex*,Vertex*>p2) const {
    if (p1.first->getIndex() == p2.first->getIndex() && p1.second->getIndex() == p2.second->getIndex())
      return true;
    return false;
  }
};



// ===================================================================================
// PARENT/CHILD VERTEX relationships (for subdivision) are stored in a
// hash table using a simple hash function based on the indices of the
// parent vertices, smaller index first
// ===================================================================================

inline size_t unordered_two_int_hash(size_t a, size_t b) {
  assert (a != b);
  if (b < a) {
    return ordered_two_int_hash(b,a);
  } else {
    assert (a < b);
    return ordered_two_int_hash(a,b);
  }
}

struct unorderedvertexpairhash {
  size_t operator()(std::pair<Vertex*,Vertex*> p) const {
    return unordered_two_int_hash(p.first->getIndex(),p.second->getIndex());
  }
};

struct unorderedsamevertexpair {
  bool operator()(std::pair<Vertex*,Vertex*> p1, std::pair<Vertex*,Vertex*>p2) const {
    if ((p1.first->getIndex() == p2.first->getIndex() && p1.second->getIndex() == p2.second->getIndex()) ||
	(p1.first->getIndex() == p2.second->getIndex() && p1.second->getIndex() == p2.first->getIndex())) return true;
    return false;
  }
};


// to handle different platforms with different variants of a developing standard
// NOTE: You may need to adjust these depending on your installation
#ifdef __APPLE__
typedef __gnu_cxx::hash_map<std::pair<Vertex*,Vertex*>,Vertex*,unorderedvertexpairhash,unorderedsamevertexpair> vphashtype;
typedef __gnu_cxx::hash_map<std::pair<Vertex*,Vertex*>,Edge*,orderedvertexpairhash,orderedsamevertexpair> edgeshashtype;
#else
#ifdef __CYGWIN__
typedef __gnu_cxx::hash_map<std::pair<Vertex*,Vertex*>,Vertex*,unorderedvertexpairhash,unorderedsamevertexpair> vphashtype;
typedef __gnu_cxx::hash_map<std::pair<Vertex*,Vertex*>,Edge*,orderedvertexpairhash,orderedsamevertexpair> edgeshashtype;
#else
// UNIX
#ifdef __FREEBSD__
// for older versions of unix without unordered maps (e.g., the submission server)
typedef __gnu_cxx::hash_map<std::pair<Vertex*,Vertex*>,Vertex*,unorderedvertexpairhash,unorderedsamevertexpair> vphashtype;
typedef __gnu_cxx::hash_map<std::pair<Vertex*,Vertex*>,Edge*,orderedvertexpairhash,orderedsamevertexpair> edgeshashtype;
#else
// newer versions 
typedef std::unordered_map<std::pair<Vertex*,Vertex*>,Vertex*,unorderedvertexpairhash,unorderedsamevertexpair> vphashtype;
typedef std::unordered_map<std::pair<Vertex*,Vertex*>,Edge*,orderedvertexpairhash,orderedsamevertexpair> edgeshashtype;
#endif
#endif
#endif


#endif // _HASH_H_
