// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TabletImage
//
//

#include "TabletImage.h"
#include "TabletImage.c7"

ClassImp(TabletImage)

/**************************************************************************/

void TabletImage::_init()
{}

/**************************************************************************/

void TabletImage::Touch(Int_t x, Int_t y, Int_t p)
{
  mTouchStamp  = Stamp(FID());
  mTouch.fTime = mTouchStamp;
  mTouch.fX    = x;
  mTouch.fY    = y;
  mTouch.fP    = p;
}

/**************************************************************************/
