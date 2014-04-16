// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuObserver.h"
#include "TringuObserver.c7"

// TringuObserver

//______________________________________________________________________________
//
//

ClassImp(TringuObserver);

//==============================================================================

void TringuObserver::_init()
{}

TringuObserver::TringuObserver(const Text_t* n, const Text_t* t) :
  ZNode(n, t)
{
  _init();
}

TringuObserver::~TringuObserver()
{}

//==============================================================================
