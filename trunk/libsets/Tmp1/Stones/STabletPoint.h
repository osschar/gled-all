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

  ClassDefNV(STabletPoint, 1);
}; // endclass STabletPoint

typedef vector<STabletPoint>            vSTabletPoint_t;
typedef vector<STabletPoint>::iterator  vSTabletPoint_i;

#endif
