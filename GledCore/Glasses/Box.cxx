// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// Box
//
//

#include "Box.h"
#include "Box.c7"

ClassImp(Box)

/**************************************************************************/

void Box::_init()
{
  // !!!! Set all links to 0 !!!!
  mA = mB = mC = 1;
}

/**************************************************************************/

void Box::SetABC(Float_t a, Float_t b, Float_t c)
{
  mA = a; mB = b; mC = c;
  Stamp(LibID(), ClassID());
}
