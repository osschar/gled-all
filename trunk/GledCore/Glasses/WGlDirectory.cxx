// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// WGlDirectory
//
//

#include "WGlDirectory.h"
#include "WGlDirectory.c7"

ClassImp(WGlDirectory)

/**************************************************************************/

void WGlDirectory::_init()
{
  // *** Set all links to 0 ***
  mContents  = 0;
  mSelection = 0;

  mStepMode = SM_XYZ;
  mDx = mDy = mDz = 1;
  mNx = mNy = mNz = 16;

  bDrawBox = true;
  mBoxOx = mBoxOy = mBoxOz = 0;
  mBoxDx = mBoxDy = mBoxDz = 1;

  bDrawText   = true;
  mTextOx = mTextOy = mTextOz = 0;
  mTextDx = mTextDy = 0;
  mTextA1 = mTextA2 = mTextA3 = 0;

  mBoxColor.rgba(0.5, 0.5, 0.8);
  mSelColor.rgba(0.8, 0.5, 0.5);

  mCbackAlpha = 0;
  mCbackMethodInfo    = 0;
  mCbackBetaClassInfo = 0;
}

/**************************************************************************/

GledNS::MethodInfo* WGlDirectory::GetCbackMethodInfo()
{
  if(mCbackMethodInfo == 0)
    mCbackMethodInfo = GledNS::DeduceMethodInfo(*mCbackAlpha, mCbackMethodName);
  return mCbackMethodInfo;
}

GledNS::ClassInfo* WGlDirectory::GetCbackBetaClassInfo()
{
  if(mCbackBetaClassInfo == 0 && mCbackBetaType != "")
    mCbackBetaClassInfo = GledNS::FindClassInfo(mCbackBetaType);
  return mCbackBetaClassInfo;
}

/**************************************************************************/

void WGlDirectory::StandardPersp()
{
  SetStepMode(SM_XZY);
  SetDx(2.5); SetDy(1);  SetDz(-1);
  SetNx(6);   SetNy(16); SetNz(16);
  SetDrawBox(1);
  SetBoxOx(-0.1); SetBoxOy(-0.1); SetBoxOz(-0.21);
  SetBoxDx(2.2);  SetBoxDy(0.6);  SetBoxDz(0.2);
  SetDrawText(1);
  SetTextOx(0);   SetTextOy(0);   SetTextOz(0);
  SetTextDx(2);   SetTextDy(0);
  SetTextA1(0);   SetTextA2(0);   SetTextA3(0);
}

void WGlDirectory::StandardFixed()
{
  SetPos(0.2, 8, 0);
  SetStepMode(SM_YXZ);
  SetDx(2);  SetDy(-0.7); SetDz(1);
  SetNx(16); SetNy(10);   SetNz(16);
  SetDrawBox(0);
  SetBoxOx(0);    SetBoxOy(0);  SetBoxOz(0);
  SetBoxDx(1);    SetBoxDy(1);  SetBoxDz(1);
  SetDrawText(1);
  SetTextOx(0);   SetTextOy(0); SetTextOz(0);
  SetTextDx(1.7); SetTextDy(0);
  SetTextA1(0);   SetTextA2(0); SetTextA3(0);
}

void WGlDirectory::StandardPixel()
{
  SetStepMode(SM_YXZ);
  SetDx(110); SetDy(-36); SetDz(1);
  SetNx(16); SetNy(10); SetNz(16);
  SetDrawBox(0);
  SetBoxOx(0);   SetBoxOy(0);  SetBoxOz(0);
  SetBoxDx(1);   SetBoxDy(1);  SetBoxDz(1);
  SetDrawText(1);
  SetTextOx(0);  SetTextOy(0); SetTextOz(0);
  SetTextDx(90); SetTextDy(0);
  SetTextA1(0);  SetTextA2(0); SetTextA3(0);
}

