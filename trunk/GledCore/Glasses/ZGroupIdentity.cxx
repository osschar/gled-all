// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGroupIdentity
//
//

#include "ZGroupIdentity.h"
#include "ZGroupIdentity.c7"
#include <Glasses/ZQueen.h>

ClassImp(ZGroupIdentity)

/**************************************************************************/

void ZGroupIdentity::_init()
{
  mActiveMMEs = 0;
}

/**************************************************************************/

void ZGroupIdentity::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mActiveMMEs == 0) {
    mActiveMMEs = new ZNameMap("ActiveMMEs", GForm("ActiveMMEs of %s", GetName()));
    mQueen->CheckIn(mActiveMMEs);
    // !!!! set to Fid of MME
  }
}

/**************************************************************************/
