// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Box
//
//

#include "Box.h"

ClassImp(Box)

/**************************************************************************/

void Box::_init()
{
  // !!!! Set all links to 0 !!!!
  mA = mB = mC = 1;
}

/**************************************************************************/

void Box::SetABC(Real_t a, Real_t b, Real_t c)
{
  mA = a; mB = b; mC = c;
  Stamp(LibID(), ClassID());
}

/**************************************************************************/

#include "Box.c7"
