// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// UINodeLink
//
//

#include "UINodeLink.h"
#include "UINodeLink.c7"

#include <Glasses/TPCSegment.h>

#include <RegExp/pme.h>

ClassImp(UINodeLink)

/**************************************************************************/

  void UINodeLink::_init()
{
  // *** Set all links to 0 ***
  mActive = true;
  mHasFocus = false;

  mCbackAlpha = 0;
  mCbackBeta = 0;
  mCbackMethodInfo = 0;

  mFocusAlpha = 0;
  mFocusBeta = 0;
  mFocusMethodInfo = 0;
}

/**************************************************************************/
GledNS::MethodInfo* UINodeLink::GetCbackMethodInfo()
{
  if(mCbackMethodInfo == 0 && mCbackAlpha)
    mCbackMethodInfo = GledNS::DeduceMethodInfo(mCbackAlpha, mCbackMethodName.Data());
  return mCbackMethodInfo;
}
/**************************************************************************/
GledNS::MethodInfo* UINodeLink::GetFocusMethodInfo()
{
  if(mFocusMethodInfo == 0 && mFocusAlpha)
    mFocusMethodInfo = GledNS::DeduceMethodInfo(mFocusAlpha, mFocusMethodName.Data());
  return mFocusMethodInfo;
}
