// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuObserverSpiritio.h"
#include <Glasses/ZNode.h>
#include "TringuObserverSpiritio.c7"

// TringuObserverSpiritio

//______________________________________________________________________________
//
//

ClassImp(TringuObserverSpiritio);

//==============================================================================

void TringuObserverSpiritio::_init()
{

}

TringuObserverSpiritio::TringuObserverSpiritio(const Text_t* n, const Text_t* t) :
  Spiritio(n, t)
{
  _init();
}

TringuObserverSpiritio::~TringuObserverSpiritio()
{}

//==============================================================================

void TringuObserverSpiritio::TimeTick(Double_t t, Double_t dt)
{

}

void TringuObserverSpiritio::Install(TSPupilInfo* pi)
{
  // This is potentially a tricky one.
  // Spiritios usually operate on the Sun, but the tringu-observer
  // is somewhat special as it is supposed to run locally.
  //
  // So, probably, TSPupilInfo will have to allow for local spiritios.

}
