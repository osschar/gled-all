// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "WGlButton.h"
#include "WGlButton.c7"

//==============================================================================
// WGlButton
//==============================================================================

//__________________________________________________________________________
//
// A button that sends MIRs as callbacks.
//
// Three are three options for setting of user data:
// a) mCbackBeta   - a lens that will be sent as beta of the MIR;
// b) mCbackValue  - an int
// c) mCbackString - a string
//
// b) and c) are streamed into the argument part of the MIR as if the
// function signature is foo(<lens-args>, Int_t, const TString&). Thus,
// it is OK to have bar(<lens-args>, Int_t) but to access the string,
// both must be decalred in the callback method.
//
// Maybe b) and c) should be optional, but they are now as well, in a
// more abstract sense.

ClassImp(WGlButton);

/**************************************************************************/

void WGlButton::_init()
{
  mCbackMethodInfo = 0;
  mCbackValue      = 0;
}

/**************************************************************************/

GledNS::MethodInfo* WGlButton::GetCbackMethodInfo()
{
  if(mCbackMethodInfo == 0)
    mCbackMethodInfo = GledNS::DeduceMethodInfo(*mCbackAlpha, mCbackMethodName.Data());
  return mCbackMethodInfo;
}

/**************************************************************************/

void WGlButton::MenuEnter()
{
  mParent->RnrOffForDaughters();
  SetRnrElements(true);
}

void WGlButton::MenuExit()
{
  SetRnrElements(false);
  mParent->RnrOnForDaughters();
}

/**************************************************************************/

#include <Gled/Gled.h>

void WGlButton::ExitGled()
{
  Gled::theOne->Exit();
}
