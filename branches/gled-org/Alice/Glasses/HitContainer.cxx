// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// HitContainer
//
//

#include "HitContainer.h"
#include "HitContainer.c7"

ClassImp(HitContainer)

/**************************************************************************/

void HitContainer::_init()
{
  mNHits = 0;
  mColor.rgba(0,0,1,1);
  // *** Set all links to 0 ***
}

/**************************************************************************/

HitContainer::~HitContainer()
{
  for(vHit_i k=mHits.begin(); k!=mHits.end(); ++k) {
    delete *k;
  }
}

/**************************************************************************/
void HitContainer::Dump()
{
  for(vHit_i i=mHits.begin(); i!=mHits.end(); ++i) {
    (*i)->Dump();
  }
}

/**************************************************************************/
