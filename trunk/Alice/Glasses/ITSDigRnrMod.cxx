// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ITSDigRnrMod
//
//

#include "ITSDigRnrMod.h"
#include "ITSDigRnrMod.c7"

ClassImp(ITSDigRnrMod)

/**************************************************************************/

  void ITSDigRnrMod::_init()
{
  // *** Set all links to 0 ***
  mSDDTreshold  = 5;
  mSDDMaxVal    = 80;
  mSSDTreshold  = 2;
  mSSDMaxVal    = 100;

  mMinCol.gray(0.3); mMaxCol.gray(1);
  mColSep = 1;
  mRibbon = 0;

  bRnrFrame   = true;
  mFrameCol.gray(0.5);
}

/**************************************************************************/


/**************************************************************************/
