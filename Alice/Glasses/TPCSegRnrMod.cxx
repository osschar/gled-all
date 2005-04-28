// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TPCSegRnrMod
//
//

#include "TPCSegRnrMod.h"
#include "TPCSegRnrMod.c7"

ClassImp(TPCSegRnrMod)

/**************************************************************************/

  void TPCSegRnrMod::_init()
{
  // *** Set all links to 0 ***
  bShowMax = true;

  mTime      = 0;
  mTimeWidth = 1;
  mTreshold  = 5;
  mMaxVal    = 80;

  mMinCol.gray(0.3); mMaxCol.gray(1);
  mColSep = 1;
  mRibbon = 0;

  bUseTexture = true;
  bRnrFrame   = true;
  mFrameCol.gray(0.5);
  mAlpha = 1.0;
}

/**************************************************************************/


/**************************************************************************/
