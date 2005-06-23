// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Cylinder
//
// Cylinder along z-axis from z=-mHeight/2 -> z=mHeight/2.
// Specify In(ner) / Out(er) radii at Base (-z) and Top (+z).
// bRnrDisks selects rendering of end-caps.
// Independent of mLodPhi, a corner of the base polygon is pointing towards
// the positive y-axis.

#include "Cylinder.h"
#include "Cylinder.c7"

ClassImp(Cylinder)

void Cylinder::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;

  mOrientation = O_Z;
  mPhiOffset   = 0;

  mHeight = 2;
  bRnrDisks = true;

  mROutBase = mROutTop = 1.0;
  mRInBase  = mRInTop  = 0; 

  mLodH     = 1;
  mLodPhi   = 8;
}
