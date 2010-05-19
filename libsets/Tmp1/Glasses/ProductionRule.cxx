// $Id: Glass_SKEL.cxx 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ProductionRule.h"
#include "ProductionRule.c7"

// ProductionRule

//______________________________________________________________________________
//
//

ClassImp(ProductionRule);

//==============================================================================

void ProductionRule::_init()
{}

ProductionRule::ProductionRule(const Text_t* n, const Text_t* t) :
  ZGlass(n, t),
  mConsumer(0)
{
  _init();
  mRule = t;
}

ProductionRule::~ProductionRule()
{}

//==============================================================================
void ProductionRule::SetRule(const Text_t* t)
{
  mRule = t;
  mConsumer->ReTriangulate();
}
