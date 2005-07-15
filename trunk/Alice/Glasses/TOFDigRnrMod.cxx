// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TOFDigRnrMod
//
//

#include "TOFDigRnrMod.h"
#include "TOFDigRnrMod.c7"

ClassImp(TOFDigRnrMod)

/**************************************************************************/

  void TOFDigRnrMod::_init()
{
  // *** Set all links to 0 *** 
  bRnrFlat = true; 

  bRnrModFrame = false;
  bRnrStripFrame = true;
  bRnrEmptyStrip = false;
  mStripCol.gray(0.4);
  mPlateCol.gray(0.8);

  mMinCol.gray(0.3); mMaxCol.gray(1);
  mColSep = 1;
  mTdcScale = 0.1;
  mTdcMinH = 0.2;
  mRibbon = 0;
}

/**************************************************************************/


/**************************************************************************/
