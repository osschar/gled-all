// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_XXXCart_H
#define Var1_XXXCart_H

#include <Glasses/Crawler.h>

#include <Stones/SVars.h>

class XXXCart : public Crawler
{
  MAC_RNR_FRIENDS(XXXCart);

private:
  void _init();

protected:
  SDesireVarF   mWheel;    // X{RGSD} 7 DesireVar(-range=>[-5, 5, 1, 50])
  SDesireVarF   mThrottle; // X{RGSD} 7 DesireVar(-range=>[0, 100, 1, 10])

  Int_t         mTest;     // X{GD}

public:
  XXXCart(const Text_t* n="XXXCart", const Text_t* t=0);
  virtual ~XXXCart();

  virtual void TimeTick(Double_t t, Double_t dt);

#include "XXXCart.h7"
  ClassDef(XXXCart, 1);
}; // endclass XXXCart

#endif
