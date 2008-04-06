// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Cylinder_H
#define GledCore_Cylinder_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

class Cylinder : public ZNode {
  MAC_RNR_FRIENDS(Cylinder);

public:
  enum Orientation_e { O_X, O_Y, O_Z };
private:
  void _init();

protected:
  Orientation_e mOrientation;   // X{GST}  7 PhonyEnum(-join=>1)
  Float_t       mPhiOffset;     // X{GST}  7 Value(-range=>[-0.5,0.5, 1,1000])

  Float_t       mHeight;        // X{GST}  7 Value(-range=>[0,1000,1,1000], -join=>1)
  Bool_t        bRnrDisks;      // X{gST}  7 Bool()
  Float_t	mRInBase;	// X{GST}  7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t	mRInTop;	// X{GST}  7 Value(-range=>[0,1000,1,1000])
  Float_t	mROutBase;	// X{GST}  7 Value(-range=>[0,1000,1,1000], -join=>1)
  Float_t	mROutTop;	// X{GST}  7 Value(-range=>[0,1000,1,1000])
  ZColor	mColor;		// X{PGST} 7 ColorButt()
  Int_t		mLodH;		// X{GST}  7 Value(-range=>[1,100,1,1], -join=>1)
  Int_t		mLodPhi;	// X{GST}  7 Value(-range=>[1,100,1,1])

public:
  Cylinder(const Text_t* n="Cylinder", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

#include "Cylinder.h7"
  ClassDef(Cylinder, 1) // Simple cylinder, possibly hollow.
}; // endclass Cylinder


#endif
