// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_Rect_H
#define Geom1_Rect_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

class Rect : public ZNode {
  MAC_RNR_FRIENDS(Rect);

private:
  void _init();

protected:
  Float_t	mULen;		// X{GS}  7 Value(-range=>[0,1000, 1,1000], -join=>1)
  Float_t	mVLen;		// X{GS}  7 Value(-range=>[0,1000, 1,1000])
  Short_t	mUStrips;	// X{GS}  7 Value(-range=>[0,10000,1], -join=>1)
  Short_t	mVStrips;	// X{GS}  7 Value(-range=>[0,10000,1])
  Float_t	mWidth;		// X{GS}  7 Value(-range=>[0,128,1,100])
  ZColor	mColor;		// X{PGS} 7 ColorButt()

public:
  Rect(const Text_t* n="Rect", const Text_t* t=0) : ZNode(n,t) { _init(); }

  void SetUV(Float_t u, Float_t v)
  { mULen = u; mVLen = v; Stamp(FID()); }
  void SetUnitSquare(UShort_t size)
  { mULen = mVLen = size; mUStrips = mVStrips = size; Stamp(FID()); }

#include "Rect.h7"
  ClassDef(Rect, 1);
}; // endclass Rect


#endif
