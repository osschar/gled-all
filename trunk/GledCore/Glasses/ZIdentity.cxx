// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZIdentity
//
// A glass representation of a user identity. ZGroupIdentity
// represents user groups and/or virtual organizations.

#include "ZIdentity.h"
#include "ZIdentity.c7"
#include <Glasses/ZQueen.h>

ClassImp(ZIdentity)

/**************************************************************************/

void ZIdentity::_init()
{
  mGlassBits |= ZGlassBits::kFixedName;
  mActiveMEEs = 0;
  mAllowThis = 0;
}

/**************************************************************************/

void ZIdentity::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mActiveMEEs == 0) {
    mActiveMEEs = new ZHashList("ActiveMEEs", GForm("ActiveMEEs of %s", GetName()));
    mActiveMEEs->SetElementFID(ZMirEmittingEntity::FID());
    mQueen->CheckIn(mActiveMEEs.get());
  }
}

/**************************************************************************/
