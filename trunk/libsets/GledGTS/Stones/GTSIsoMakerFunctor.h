// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledGTS_GTSIsoMakerFunctor_H
#define GledGTS_GTSIsoMakerFunctor_H

#include "Stones/HTrans.h"

class GTSIsoMaker;

class GTSIsoMakerFunctor
{
public:
  virtual ~GTSIsoMakerFunctor() {}

  virtual void     GTSIsoBegin(GTSIsoMaker* maker, Double_t iso_value) {}
  virtual Double_t GTSIsoFunc(Double_t x, Double_t y, Double_t z) = 0;
  virtual Double_t GTSIsoGradient(Double_t x, Double_t y, Double_t z, HPointD& g) = 0;
  virtual void     GTSIsoEnd() {}

#include "GTSIsoMakerFunctor.h7"
  ClassDef(GTSIsoMakerFunctor, 0);
}; // endclass GTSIsoMakerFunctor

#endif
