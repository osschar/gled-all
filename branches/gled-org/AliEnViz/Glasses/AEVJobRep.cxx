// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVJobRep
//
// Minimal job representation.
// Collected data from mona's ALIEN_QUERY.

#include "AEVJobRep.h"
#include "AEVJobRep.c7"

ClassImp(AEVJobRep)

/**************************************************************************/

void AEVJobRep::_init()
{}

/**************************************************************************/

void AEVJobRep::FormatTitle()
{
  AEVJobRep::SetTitle(GForm("'%-8s' by %-8s: %-10s -- %s",
			    mJobname.Data(), mUsername.Data(), mStatus.Data(),
			    mDateStr.Data()));
}

/**************************************************************************/
