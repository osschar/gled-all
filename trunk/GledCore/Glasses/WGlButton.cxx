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

#include <RegExp/pme.h>

ClassImp(WGlButton)

/**************************************************************************/

void WGlButton::_init()
{
  // *** Set all links to 0 ***
  mDx = mDy = 0;
}

/**************************************************************************/

GledNS::MethodInfo* WGlButton::GetCbackMethodInfo()
{
  using namespace GledNS;

  if(mCbackMethodInfo == 0) {
    PME re("::");
    int ret = re.split(mCbackMethodName.Data());
    if(ret == 2) {
      ClassInfo* ci = FindClassInfo(re[0]);
      if(ci == 0)  return 0;
      mCbackMethodInfo = ci->FindMethodInfo(re[1], false);
    }
    else if(ret == 1) {
      ClassInfo* ci = mCbackAlpha->VGlassInfo();
      mCbackMethodInfo = ci->FindMethodInfo(re[0], true);
    }
  }
  return mCbackMethodInfo;
}

/**************************************************************************/
