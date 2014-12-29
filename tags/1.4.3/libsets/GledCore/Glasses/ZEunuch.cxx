// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZEunuch
//
//

#include "ZEunuch.h"

#include "ZEunuch.c7"

ClassImp(ZEunuch);

/**************************************************************************/

void ZEunuch::_init()
{
  mPrimQueen = 0;
  mSecQueen  = 0;
  mToSaturn = 0;

  mRequest = RT_Undef;
  mPushStrategy = PS_Undef;
}

/**************************************************************************/


/**************************************************************************/
