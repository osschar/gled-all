// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XXXCart.h"
#include "XXXCart.c7"

// XXXCart

//______________________________________________________________________________
//
//

ClassImp(XXXCart);

//==============================================================================

void XXXCart::_init()
{
  mWheel   .SetMinMaxDelta(-5, 5, 2.5);
  mThrottle.SetMinMaxDelta(0, 100, 10);
}

XXXCart::XXXCart(const Text_t* n, const Text_t* t) :
  Crawler(n, t)
{
  _init();
}

XXXCart::~XXXCart()
{}

//==============================================================================

void XXXCart::TimeTick(Double_t t, Double_t dt)
{
  mWheel.   TimeTick(dt);
  mThrottle.TimeTick(dt);

  PARENT_GLASS::TimeTick(t, dt);
}
