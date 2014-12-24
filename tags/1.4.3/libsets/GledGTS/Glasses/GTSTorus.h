// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledGTS_GTSTorus_H
#define GledGTS_GTSTorus_H

#include <Glasses/GTSurf.h>
#include <Stones/GTSIsoMakerFunctor.h>

class GTSTorus : public GTSurf,
		 public GTSIsoMakerFunctor
{
  MAC_RNR_FRIENDS(GTSTorus);

private:
  void _init();

  Double_t  mRminvsqr; //!

protected:
  Double_t  mRM;   // X{GS} 7 Value(-range=>[0, 1000, 1, 1000], -join=>1)
  Double_t  mRm;   // X{GS} 7 Value(-range=>[0, 1000, 1, 1000], -join=>1)
  Double_t  mStep; // X{GS} 7 Value(-range=>[0, 100,  1, 1e6])

public:
  GTSTorus(const Text_t* n="GTSTorus", const Text_t* t=0);
  virtual ~GTSTorus();

  // --- GTSIsoMakerFunctor ---
  virtual void     GTSIsoBegin(GTSIsoMaker* maker, Double_t iso_value);
  virtual Double_t GTSIsoFunc(Double_t x, Double_t y, Double_t z);
  virtual Double_t GTSIsoGradient(Double_t x, Double_t y, Double_t z, HPointD& g);
  virtual void     GTSIsoEnd();

#include "GTSTorus.h7"
  ClassDef(GTSTorus, 1);
}; // endclass GTSTorus

#endif
