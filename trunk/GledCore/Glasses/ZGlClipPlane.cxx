// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGlClipPlane
//
//

#include "ZGlClipPlane.h"
#include "ZGlClipPlane.c7"

ClassImp(ZGlClipPlane)

/**************************************************************************/

void ZGlClipPlane::_init()
{
  mX = mY = mZ = 0;
  mDist = mTheta = mPhi = 0;

  bRnrSelf = true; bOnIfOff = true; bOffIfOn = false;
}

/**************************************************************************/


/**************************************************************************/
