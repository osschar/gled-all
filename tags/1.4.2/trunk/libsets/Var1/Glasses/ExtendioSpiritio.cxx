// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ExtendioSpiritio.h"
#include <Glasses/Extendio.h>
#include "ExtendioSpiritio.c7"

// ExtendioSpiritio

//______________________________________________________________________________
//
//

ClassImp(ExtendioSpiritio);

//==============================================================================

void ExtendioSpiritio::_init()
{}

ExtendioSpiritio::ExtendioSpiritio(const Text_t* n, const Text_t* t) :
  Spiritio(n, t)
{
  _init();
}

ExtendioSpiritio::~ExtendioSpiritio()
{}

//==============================================================================
