// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZMirEmittingEntity.h"
#include "ZMirEmittingEntity.c7"
#include <Glasses/ZQueen.h>

//__________________________________________________________________________
// ZMirEmittingEntity
//
//

ClassImp(ZMirEmittingEntity);

/**************************************************************************/

void ZMirEmittingEntity::_init()
{
  mPrimaryIdentity  = 0;
  mActiveIdentities = 0;
}

/**************************************************************************/

void ZMirEmittingEntity::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mActiveIdentities == 0) {
    assign_link<ZHashList>(mActiveIdentities, FID(), "ActiveIdentities",
                           GForm("ActiveIdentities of %s", GetName()));
    mActiveIdentities->SetElementFID(ZIdentity::FID());
  }
}

/**************************************************************************/

Bool_t ZMirEmittingEntity::HasIdentity(ZIdentity* ident)
{
  return ( mPrimaryIdentity == ident ||
	  (mActiveIdentities != 0 && mActiveIdentities->Has(ident))
	 );
}

/**************************************************************************/
