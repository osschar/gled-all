// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Airplane.h"
#include "Airplane.c7"

// Airplane

//______________________________________________________________________________
//
//

ClassImp(Airplane);

//==============================================================================

void Airplane::_init()
{}

Airplane::Airplane(const Text_t* n, const Text_t* t) :
  Flyer(n, t)
{
  _init();
}

Airplane::~Airplane()
{}

//==============================================================================