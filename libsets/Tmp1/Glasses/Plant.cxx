// $Id: Glass_SKEL.cxx 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Plant.h"
#include "Plant.c7"

// Plant

//______________________________________________________________________________
//
//

ClassImp(Plant);

//==============================================================================

void Plant::_init()
{}

Plant::Plant(const Text_t* n, const Text_t* t) :
Weed(n, t),
mStemWidth(0.015),
mLeafSize(0.5),
mFlowerSize(0.4)
{
  _init();
}

Plant::~Plant()
{}

//==============================================================================
