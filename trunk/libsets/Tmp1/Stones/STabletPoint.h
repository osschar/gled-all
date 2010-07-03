// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_STabletPoint_H
#define Tmp1_STabletPoint_H

#include "Stones/HTrans.h"

#include <vector>

class STabletPoint : public HPointF
{
public:
  Float_t t, p;

  STabletPoint() : HPointF(), t(0), p(0) {}
  STabletPoint(Float_t _x, Float_t _y, Float_t _z, Float_t _t, Float_t _p) :
    HPointF(_x, _y, _z), t(_t), p(_p) {}

  STabletPoint& operator+=(const STabletPoint& a) { HPointF::operator+=(a); t += a.t; p += a.p; return *this; }
  STabletPoint& operator-=(const STabletPoint& a) { HPointF::operator-=(a); t -= a.t; p -= a.p; return *this; }
  STabletPoint& operator*=(Float_t f)             { HPointF::operator*=(f); t *= f;   p *= f;   return *this; }

  void Print() const;

  ClassDefNV(STabletPoint, 1);
}; // endclass STabletPoint

//------------------------------------------------------------------------------

typedef vector<STabletPoint>            vSTabletPoint_t;
typedef vector<STabletPoint>::iterator  vSTabletPoint_i;

//------------------------------------------------------------------------------

inline STabletPoint operator+(const STabletPoint& a, const STabletPoint& b)
{
   STabletPoint r(a);
   return r += b;
}

inline STabletPoint operator-(const STabletPoint& a, const STabletPoint& b)
{
   STabletPoint r(a);
   return r -= b;
}

inline STabletPoint operator*(const STabletPoint& a, Float_t b)
{
   STabletPoint r(a);
   return r *= b;
}

inline STabletPoint operator*(Float_t b, const STabletPoint& a)
{
   STabletPoint r(a);
   return r *= b;
}

#endif
