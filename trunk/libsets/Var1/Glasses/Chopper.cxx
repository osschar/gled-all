// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Chopper.h"
#include "Chopper.c7"

// Chopper

//______________________________________________________________________________
//
//

ClassImp(Chopper);

//==============================================================================

void Chopper::_init()
{}

Chopper::Chopper(const Text_t* n, const Text_t* t) :
  Flyer(n, t)
{
  _init();
}

Chopper::~Chopper()
{}

//==============================================================================
