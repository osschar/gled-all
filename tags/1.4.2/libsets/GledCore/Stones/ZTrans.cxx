// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZTrans.h"

//______________________________________________________________________
// ZTrans
//
// ZTrans is a 4x4 transformation matrix for homogeneous coordinates
// stored internaly in a column-major order to allow direct usage by
// GL. The element type is Double32_t as statically the floats would
// be precise enough but continuous operations on the matrix must
// retain precision of column vectors.
//
// Cartan angles in mA[1-3] (+z, -y, +x) are stored for backward
// compatibility and will probably be removed soon.
//
// Direct  element access (first two should be used with care):
// operator[i]    direct access to elements,   i:0->15
// CM(i,j)        element 4*j + i;           i,j:0->3    { CM ~ c-matrix }
// operator(i,j)  element 4*(j-1) + i - 1    i,j:1->4
//
// Column-vector access:
// USet Get/SetBaseVec(), Get/SetPos() and Arr[XYZT]() methods.
//
// For all methods taking the matrix indices:
// 1->X, 2->Y, 3->Z; 4->Position (if applicable). 0 reserved for time.
//
// Shorthands in method-names:
// LF ~ LocalFrame; PF ~ ParentFrame; IP ~ InPlace

ClassImp(ZTrans);

/**************************************************************************/

//ZTrans::ZTrans(const Double_t arr[16]) { SetFromArray(arr); }

//ZTrans::ZTrans(const Float_t  arr[16]) { SetFromArray(arr); }

ZTrans ZTrans::operator*(const ZTrans& t)
{
  ZTrans b(*this);
  b.MultRight(t);
  return b;
}
