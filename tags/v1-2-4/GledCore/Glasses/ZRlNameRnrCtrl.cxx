// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZRlNameRnrCtrl
//
//

#include "ZRlNameRnrCtrl.h"

#include "ZRlNameRnrCtrl.c7"

ClassImp(ZRlNameRnrCtrl)

/**************************************************************************/

void ZRlNameRnrCtrl::_init()
{
  // !!!! Set all links to 0 !!!!
  mNameRnrOp  = GSO_Nop;
  bRnrTiles   = true; bRnrFrames = true;
  mNameOffset = 0.99;
  mTextSize   = 12;
  mTextCol.rgba(1, 1, 1);
  mTileCol.rgba(0, 0, 0.3);
  mTilePos = "";
}

/**************************************************************************/


/**************************************************************************/
