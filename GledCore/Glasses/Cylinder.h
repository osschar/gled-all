// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Cylinder_H
#define GledCore_Cylinder_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

class Cylinder : public ZNode {
  MAC_RNR_FRIENDS(Cylinder);

private:
  void _init();

protected:
  Float_t       mHeight;        // X{GS}  7 Value(-range=>[0,1000,1,1000], -join=>1)
  Bool_t        bRnrDisks;      // X{gS}  7 Bool()
  Float_t	mRInBase;	// X{GS}  7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t	mRInTop;	// X{GS}  7 Value(-range=>[0,1000,1,1000])
  Float_t	mROutBase;	// X{GS}  7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t	mROutTop;	// X{GS}  7 Value(-range=>[0,1000,1,1000])
  ZColor	mColor;		// X{PGS} 7 ColorButt()
  Int_t		mLodZ;		// X{GS}  7 Value(-range=>[1,100,1,1], -join=>1)
  Int_t		mLodPhi;	// X{GS}  7 Value(-range=>[1,100,1,1])

public:
  Cylinder(const Text_t* n="Cylinder", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

#include "Cylinder.h7"
  ClassDef(Cylinder, 1) // Simple cylinder, possibly hollow.
}; // endclass Cylinder

GlassIODef(Cylinder);

#endif
