#ifndef GLED_GCC_CONFIG_H
#define GLED_GCC_CONFIG_H

/**************************************************************************/
#if __GNUC__ == 2
/**************************************************************************/

#include <strstream>

#ifndef __CINT__

#include <hash_map>

template <>
struct hash<string> {
  hash<const char *> h;  
  size_t operator()(const string& str) const {
      return (h(str.c_str()));
  }
};

template <class T>
struct hash<T*> {
  size_t operator()(const T* s) const {
      hash<UInt_t> h; return h((UInt_t)s);
  }
};

#endif

/**************************************************************************/
#elif __GNUC__ == 3
/**************************************************************************/

#include <sstream>

#ifndef __CINT__
#include <ext/hash_map>

using namespace __gnu_cxx;

// !! krumph ... one would expect this in stdlib ... perhaps gcc-3
namespace __gnu_cxx {
template <>
struct hash<string> {
  hash<const char *> h;  
  size_t operator()(const string& str) const {
      return (h(str.c_str()));
  }
};
// this one appears semi-obvious as well ...
template <class T>
struct hash<T*> {
  size_t operator()(const T* s) const {
      hash<UInt_t> h; return h((UInt_t)s);
  }
};
}
#endif

/**************************************************************************/
#else
/**************************************************************************/

#error "Unknown gcc major version"

/**************************************************************************/
#endif
/**************************************************************************/

#endif
