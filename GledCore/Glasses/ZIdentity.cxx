// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZIdentity
//
//

#include "ZIdentity.h"

ClassImp(ZIdentity)

/**************************************************************************/

void ZIdentity::_init()
{
  // !!!! Set all links to 0 !!!!
  mGlassBits |= ZGlassBits::kFixedName;
  mNumMMEs = 0;
  mAllowThis = 0;
}

/**************************************************************************/



/**************************************************************************/

#include "ZIdentity.c7"
