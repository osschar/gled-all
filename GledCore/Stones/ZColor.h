// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZColor_H
#define GledCore_ZColor_H

#include <Gled/GledTypes.h>

class ZColor
{
protected:
  Float_t mC[4];

public:
  ZColor()             { mC[0] = 1; mC[1] = 1; mC[2] = 1; mC[3] = 1;}
  ZColor(Float_t gray) { mC[0] = mC[1] = mC[2] = gray; mC[3]=1; }
  ZColor(Float_t r, Float_t g, Float_t b, Float_t a = 1)
  { mC[0]=r; mC[1]=g; mC[2]=b; mC[3]=a; }
  virtual ~ZColor() {}

  Float_t r() const { return mC[0]; }
  Float_t g() const { return mC[1]; }
  Float_t b() const { return mC[2]; }
  Float_t a() const { return mC[3]; }
  Float_t gray() const { return (mC[0]+mC[1]+mC[2])/3; }

  void r(Float_t x) { mC[0] = x; }
  void g(Float_t x) { mC[1] = x; }
  void b(Float_t x) { mC[2] = x; }
  void a(Float_t x) { mC[3] = x; }
  void rgba(Float_t r, Float_t g, Float_t b, Float_t a = 1)
  { mC[0]=r; mC[1]=g; mC[2]=b; mC[3]=a; }
  void gray(Float_t gray, Float_t a = 1)
  { mC[0]=mC[1]=mC[2]=gray; mC[3]=a; }

  void rgba_from_ubyte(UChar_t r, UChar_t g, UChar_t b, UChar_t a = 255)
  { Float_t f=1.0f/255; mC[0]=f*r; mC[1]=f*g; mC[2]=f*b; mC[3]=f*a; }
  void rgba_from_ubyte(UChar_t* c)
  { Float_t f=1.0f/255; mC[0]=f*c[0]; mC[1]=f*c[1]; mC[2]=f*c[2]; mC[3]=f*c[3]; }

  void to_ubyte(UChar_t* ub, bool wrap=false) const;
  void rgb_to_ubyte(UChar_t* ub, bool wrap=false) const;

  const Float_t* operator()() const { return mC; }
  const Float_t* array()      const { return mC; }
  Float_t  operator[](int i) const { return mC[i]; }
  Float_t& operator[](int i) { return mC[i]; }

  ZColor& operator*=(Float_t a) {
    mC[0]*=a; mC[1]*=a; mC[2]*=a; mC[3]*=a;
    return *this;
  }

  ZColor& operator+=(ZColor& x) {
    mC[0] += x[0]; mC[1] += x[1]; mC[2] += x[2]; mC[3] += x[3];
    return *this;
  }

  friend ZColor operator*(Float_t a, const ZColor& x);
  friend ZColor operator+(const ZColor& x, const ZColor& y);
  friend ZColor operator-(const ZColor& x, const ZColor& y);
  friend ZColor operator/(const ZColor& x, const ZColor& y);

  friend ostream& operator<<(ostream& s, ZColor& c);

  ClassDef(ZColor,1);
};

/**************************************************************************/

inline void ZColor::to_ubyte(UChar_t* ub, bool wrap) const
{
  if(wrap) {
    for(int i=0; i<4; ++i)
      ub[i] = (UChar_t)(255*mC[i]);
  } else {
    for(int i=0; i<4; ++i)
      ub[i] = mC[i] >= 1 ? 255 : (mC[i] <= 0 ? 0 : (UChar_t)(255*mC[i]));
  }
}

inline void ZColor::rgb_to_ubyte(UChar_t* ub, bool wrap) const
{
  if(wrap) {
    for(int i=0; i<3; ++i)
      ub[i] = (UChar_t)(255*mC[i]);
  } else {
    for(int i=0; i<3; ++i)
      ub[i] = mC[i] >= 1 ? 255 : (mC[i] <= 0 ? 0 : (UChar_t)(255*mC[i]));
  }
}

/**************************************************************************/

ostream& operator<<(ostream& s, ZColor& c);

inline ZColor operator*(float a, const ZColor& x) {
  return ZColor(a*x.mC[0], a*x.mC[1], a*x.mC[2], a*x.mC[3]);
}

inline ZColor operator+(const ZColor& x, const ZColor& y) {
  return ZColor(x.mC[0]+y.mC[0], x.mC[1]+y.mC[1], x.mC[2]+y.mC[2], x.mC[3]+y.mC[3]);
}

inline ZColor operator-(const ZColor& x, const ZColor& y) {
  return ZColor(x.mC[0]-y.mC[0], x.mC[1]-y.mC[1], x.mC[2]-y.mC[2], x.mC[3]-y.mC[3]);
}

inline ZColor operator/(const ZColor& x, const ZColor& y) {
  // Average color
  return ZColor((x.mC[0]+y.mC[0])/2, (x.mC[1]+y.mC[1])/2,
		(x.mC[2]+y.mC[2])/2, (x.mC[3]+y.mC[3])/2);
}

#endif
