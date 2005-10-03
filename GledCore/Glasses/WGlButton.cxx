// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// WGlButton
//
//

#include "WGlButton.h"
#include "WGlButton.c7"

ClassImp(WGlButton)

/**************************************************************************/

void WGlButton::_init()
{
  // *** Set all links to 0 ***
  mDx = mDy = 0;
  mCbackAlpha = 0;
  mCbackMethodInfo = 0;
}

/**************************************************************************/

GledNS::MethodInfo* WGlButton::GetCbackMethodInfo()
{
  if(mCbackMethodInfo == 0)
    mCbackMethodInfo = GledNS::DeduceMethodInfo(*mCbackAlpha, mCbackMethodName.Data());
  return mCbackMethodInfo;
}

/**************************************************************************/
