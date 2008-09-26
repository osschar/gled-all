// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVCompElRep
//
//

#include "AEVCompElRep.h"

#include "AEVCompElRep.c7"

ClassImp(AEVCompElRep)

/**************************************************************************/

void AEVCompElRep::_init()
{
  mNSlots = mNSFree = 100;
  mSpeedFacAvg = 1; mSpeedFacSgm = 0.02;
}

/**************************************************************************/


/**************************************************************************/
