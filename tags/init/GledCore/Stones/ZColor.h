// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZColor_H
#define Gled_ZColor_H

#include <Gled/GledTypes.h>
#include <TObject.h>

class ZColor {
protected:
  Float_t c[4];
public:

  ZColor() { c[0]=1; c[1]=1; c[2]=1; c[3]=1;}
  ZColor(Float_t r, Float_t g, Float_t b, Float_t a = 1) {
    c[0]=r; c[1]=g; c[2]=b; c[3]=a;
  }

  Float_t r() const { return c[0]; }
  Float_t g() const { return c[1]; }
  Float_t b() const { return c[2]; }
  Float_t a() const { return c[3]; }
  void r(Float_t x) { c[0] = x; }
  void g(Float_t x) { c[1] = x; }
  void b(Float_t x) { c[2] = x; }
  void a(Float_t x) { c[3] = x; }
  void rgba(Float_t r, Float_t g, Float_t b, Float_t a = 1) {
    c[0]=r; c[1]=g; c[2]=b; c[3]=a;
  }
  void gray(Float_t gray, Float_t a = 1) {
    c[0]=c[1]=c[2]=gray; c[3]=a;
  }

  Float_t* operator()() { return c; }
  Float_t  operator[](int i) const { return c[i]; }
  Float_t& operator[](int i) { return c[i]; }

  friend ZColor operator*(Float_t a, const ZColor& x);
  friend ZColor operator+(const ZColor& x, const ZColor& y);
  friend ZColor operator-(const ZColor& x, const ZColor& y);
  friend ZColor operator/(const ZColor& x, const ZColor& y);

  friend ostream& operator<<(ostream& s, ZColor& c);

  ClassDef(ZColor,1)
};

ostream& operator<<(ostream& s, ZColor& c);

inline ZColor operator*(float a, const ZColor& x) {
  return ZColor(a*x.c[0], a*x.c[1], a*x.c[2], a*x.c[3]);
}

inline ZColor operator+(const ZColor& x, const ZColor& y) {
  return ZColor(x.c[0]+y.c[0], x.c[1]+y.c[1], x.c[2]+y.c[2], x.c[3]+y.c[3]);
}

inline ZColor operator-(const ZColor& x, const ZColor& y) {
  return ZColor(x.c[0]-y.c[0], x.c[1]-y.c[1], x.c[2]-y.c[2], x.c[3]-y.c[3]);
}

inline ZColor operator/(const ZColor& x, const ZColor& y) {
  // Average color
  return ZColor((x.c[0]+y.c[0])/2, (x.c[1]+y.c[1])/2,
		(x.c[2]+y.c[2])/2, (x.c[3]+y.c[3])/2);
}

#endif
