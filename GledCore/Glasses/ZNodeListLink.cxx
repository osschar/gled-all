// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZNodeListLink
//
//

#include "ZNodeListLink.h"
#include "ZNodeListLink.c7"

#include <RegExp/pme.h>

ClassImp(ZNodeListLink)

/**************************************************************************/

void ZNodeListLink::_init()
{
  // *** Set all links to 0 ***
  mContents  = 0;
  mCurrent   = 0;
  mSelection = 0;

  mViewMode = VM_Nop;
  mOrthoW    = 10; mOrthoH   = 10;
  mOrthoNear = -1; mOrthoFar = 1;

  mStepMode = SM_XYZ;
  mOx = mOy = mOz = 0;
  mDx = mDy = mDz = 1;
  mNx = mNy = mNz = 16;

  bDrawBox = true;
  mBoxDx = mBoxDy = mBoxDz = 0;
  mBoxSx = mBoxSy = mBoxSz = 1;

  bDrawText   = true;
  mTextYSize  = 0.6;
  bTextCenter = false;
  mTextMaxLen = 6;
  mTextFadeW  = 1.5;
  mTextDx = mTextDy = mTextDz = 0;
  mTextA1 = mTextA2 = mTextA3 = 0;

  bDrawTile = false; bFullTile = false; bDrawFrame = false;
  mXBorder = mYBorder = 0.1;
  
  mTextColor.rgba(1, 1, 1);
  mBoxColor.rgba(0.5, 0.5, 0.8);
  mSelColor.rgba(0.8, 0.5, 0.5);
  mCurColMod.rgba(0.1, 0.1, 0.1, 0);

  mCbackAlpha = 0;
  mCbackMethodInfo = 0;
}

/**************************************************************************/

GledNS::MethodInfo* ZNodeListLink::GetCbackMethodInfo()
{
  using namespace GledNS;

  if(mCbackMethodInfo == 0) {
    PME re("::");
    int ret = re.split(mCbackMethodName.Data());
    if(ret != 2) return 0;
    ClassInfo* ci = FindClassInfo(re[0]);
    if(ci == 0)  return 0;
    mCbackMethodInfo = ci->FindMethodInfo(re[1], true);
  }
  return mCbackMethodInfo;
}

/**************************************************************************/

void ZNodeListLink::XPrint()
{
  // Dumps code to restore rendering mode.
  // Should be done in some other way.

  printf("ZNodeListLink& X;\n");

  printf("X.SetViewMode((ZNodeListLink::ViewMode_e)%d);\n", mViewMode);
  printf("X.SetOrthoW(%f); X.SetOrthoH(%f);\n", mOrthoW, mOrthoH);
  printf("X.SetOrthoNear(%f); X.SetOrthoFar(%f);\n", mOrthoNear, mOrthoFar);

  printf("X.SetStepMode((ZNodeListLink::StepMode_e)%d);\n", mStepMode);
  printf("X.SetOx(%f); X.SetOy(%f); X.SetOz(%f);\n", mOx, mOy, mOz);
  printf("X.SetDx(%f); X.SetDy(%f); X.SetDz(%f);\n", mDx, mDy, mDz);
  printf("X.SetNx(%d); X.SetNy(%d); X.SetNz(%d);\n", mNx, mNy, mNz);

  printf("X.SetDrawBox(%d);\n", bDrawBox);
  printf("X.SetBoxDx(%f); X.SetBoxDy(%f); X.SetBoxDz(%f);\n", mBoxDx, mBoxDy, mBoxDz);
  printf("X.SetBoxSx(%f); X.SetBoxSy(%f); X.SetBoxSz(%f);\n", mBoxSx, mBoxSy, mBoxSz);

  printf("X.SetDrawText(%d);\n", bDrawText);
  printf("X.SetTextYSize(%f);\n", mTextYSize);
  printf("X.SetTextCenter(%d);\n", bTextCenter);
  printf("X.SetTextMaxLen(%f);\n", mTextMaxLen);
  printf("X.SetTextFadeW(%f);\n", mTextFadeW);
  printf("X.SetTextDx(%f); X.SetTextDy(%f); X.SetTextDz(%f);\n", mTextDx, mTextDy, mTextDz);
  printf("X.SetTextA1(%f); X.SetTextA2(%f); X.SetTextA3(%f);\n", mTextA1, mTextA2, mTextA3);

  printf("X.SetDrawTile(%d); X.SetFullTile(%d); X.SetDrawFrame(%d);\n", bDrawTile, bFullTile, bDrawFrame);
  printf("X.SetXBorder(%f); X.SetYBorder(%f);\n", mXBorder, mYBorder);
  
  
  printf("X.SetTextColor(%f, %f, %f, %f);\n", mTextColor.r(), mTextColor.g(), mTextColor.b(), mTextColor.a());
  printf("X.SetBoxColor(%f, %f, %f, %f);\n",  mBoxColor.r(), mBoxColor.g(), mBoxColor.b(), mBoxColor.a());
  printf("X.SetSelColor(%f, %f, %f, %f);\n",  mSelColor.r(), mSelColor.g(), mSelColor.b(), mSelColor.a());
  printf("X.SetCurColMod(%f, %f, %f, %f);\n", mCurColMod.r(), mCurColMod.g(), mCurColMod.b(), mCurColMod.a());
}

/**************************************************************************/

void ZNodeListLink::StandardPersp()
{
  ZNodeListLink& X = *this;
  X.SetViewMode((ZNodeListLink::ViewMode_e)0);
  X.SetOrthoW(10.000000); X.SetOrthoH(10.000000);
  X.SetOrthoNear(-1.000000); X.SetOrthoFar(1.000000);
  X.SetStepMode((ZNodeListLink::StepMode_e)0);
  X.SetOx(0.000000); X.SetOy(0.000000); X.SetOz(0.000000);
  X.SetDx(2.500000); X.SetDy(1.000000); X.SetDz(1.000000);
  X.SetNx(6); X.SetNy(16); X.SetNz(16);
  X.SetDrawBox(1);
  X.SetBoxDx(-0.100000); X.SetBoxDy(0.010000); X.SetBoxDz(-0.100000);
  X.SetBoxSx(2.200000); X.SetBoxSy(0.200000); X.SetBoxSz(0.600000);
  X.SetDrawText(1);
  X.SetTextYSize(0.600000);
  X.SetTextCenter(0);
  X.SetTextMaxLen(2.000000);
  X.SetTextFadeW(1.500000);
  X.SetTextDx(0.000000); X.SetTextDy(0.000000); X.SetTextDz(0.000000);
  X.SetTextA1(0.000000); X.SetTextA2(0.000000); X.SetTextA3(90.000000);
  X.SetDrawTile(0); X.SetFullTile(0); X.SetDrawFrame(0);
  X.SetXBorder(0.100000); X.SetYBorder(0.100000);
  X.SetTextColor(0.000000, 0.000000, 0.000000, 1.000000);
  X.SetBoxColor(0.500000, 0.500000, 0.800000, 0.800000);
  X.SetSelColor(0.800000, 0.500000, 0.500000, 1.000000);
  X.SetCurColMod(0.100000, 0.100000, 0.100000, 0.000000);
}

void ZNodeListLink::StandardFixed()
{
  ZNodeListLink& X = *this;
  X.SetViewMode((ZNodeListLink::ViewMode_e)2);
  X.SetOrthoW(10.000000); X.SetOrthoH(10.000000);
  X.SetOrthoNear(-1.000000); X.SetOrthoFar(1.000000);
  X.SetStepMode((ZNodeListLink::StepMode_e)1);
  X.SetOx(0.150000); X.SetOy(8.000000); X.SetOz(0.000000);
  X.SetDx(2.000000); X.SetDy(-0.700000); X.SetDz(1.000000);
  X.SetNx(16); X.SetNy(10); X.SetNz(16);
  X.SetDrawBox(0);
  X.SetBoxDx(0.000000); X.SetBoxDy(0.000000); X.SetBoxDz(0.000000);
  X.SetBoxSx(1.000000); X.SetBoxSy(1.000000); X.SetBoxSz(1.000000);
  X.SetDrawText(1);
  X.SetTextYSize(0.400000);
  X.SetTextCenter(0);
  X.SetTextMaxLen(1.700000);
  X.SetTextFadeW(1.500000);
  X.SetTextDx(0.000000); X.SetTextDy(0.000000); X.SetTextDz(0.000000);
  X.SetTextA1(0.000000); X.SetTextA2(0.000000); X.SetTextA3(0.000000);
  X.SetDrawTile(1); X.SetFullTile(0); X.SetDrawFrame(1);
  X.SetXBorder(0.100000); X.SetYBorder(0.100000);
  X.SetTextColor(1.000000, 1.000000, 1.000000, 1.000000);
  X.SetBoxColor(0.500000, 0.500000, 0.800000, 0.650000);
  X.SetSelColor(0.800000, 0.500000, 0.500000, 1.000000);
  X.SetCurColMod(0.100000, 0.100000, 0.100000, 0.000000);
}

void ZNodeListLink::StandardPixel()
{
  ZNodeListLink& X = *this;
  X.SetViewMode((ZNodeListLink::ViewMode_e)1);
  X.SetOrthoW(10.000000); X.SetOrthoH(10.000000);
  X.SetOrthoNear(-1.000000); X.SetOrthoFar(1.000000);
  X.SetStepMode((ZNodeListLink::StepMode_e)1);
  X.SetOx(5.000000); X.SetOy(-50.000000); X.SetOz(0.000000);
  X.SetDx(110.000000); X.SetDy(-36.000000); X.SetDz(1.000000);
  X.SetNx(16); X.SetNy(10); X.SetNz(16);
  X.SetDrawBox(0);
  X.SetBoxDx(0.000000); X.SetBoxDy(0.000000); X.SetBoxDz(0.000000);
  X.SetBoxSx(1.000000); X.SetBoxSy(1.000000); X.SetBoxSz(1.000000);
  X.SetDrawText(1);
  X.SetTextYSize(18.000000);
  X.SetTextCenter(0);
  X.SetTextMaxLen(90.000000);
  X.SetTextFadeW(1.500000);
  X.SetTextDx(0.000000); X.SetTextDy(0.000000); X.SetTextDz(0.000000);
  X.SetTextA1(0.000000); X.SetTextA2(0.000000); X.SetTextA3(0.000000);
  X.SetDrawTile(1); X.SetFullTile(1); X.SetDrawFrame(1);
  X.SetXBorder(5.000000); X.SetYBorder(2.000000);
  X.SetTextColor(0.932255, 1.000000, 0.685185, 1.000000);
  X.SetBoxColor(0.266204, 0.266204, 0.425926, 0.650000);
  X.SetSelColor(0.800000, 0.500000, 0.500000, 1.000000);
  X.SetCurColMod(0.314815, 0.314815, 0.314815, 0.000000);
}

