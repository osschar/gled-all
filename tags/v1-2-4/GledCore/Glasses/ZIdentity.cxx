// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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
  mActiveMMEs = 0;
  mAllowThis = 0;
}

/**************************************************************************/

void ZIdentity::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mActiveMMEs == 0) {
    mActiveMMEs = new ZHashList("ActiveMMEs", GForm("ActiveMMEs of %s", GetName()));
    mActiveMMEs->SetElementFID(ZMirEmittingEntity::FID());
    mQueen->CheckIn(mActiveMMEs);
  }
}

/**************************************************************************/
