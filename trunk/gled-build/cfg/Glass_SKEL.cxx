// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CLASSNAME.h"
#include "CLASSNAME.c7"

// CLASSNAME

//______________________________________________________________________________
//
//

ClassImp(CLASSNAME);

//==============================================================================

void CLASSNAME::_init()
{}

CLASSNAME::CLASSNAME(const Text_t* n, const Text_t* t) :
  BASENAME(n, t)
{
  _init();
}

CLASSNAME::~CLASSNAME()
{}

//==============================================================================
