// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZFireQueen
//
//

#include "ZFireQueen.h"

#include "ZFireQueen.c7"

ClassImp(ZFireQueen)

/**************************************************************************/

void ZFireQueen::_init()
{
  // !!!! Set all links to 0 !!!!
  mEunuchs = 0;
}

/**************************************************************************/

void ZFireQueen::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if(mEunuchs == 0) {
    ZHashList* l = new ZHashList("Eunuchs");
    //l->SetElementFID(ZEunuch()::FID());
    mQueen->CheckIn(l);
    SetEunuchs(l);
  }
}


/**************************************************************************/
