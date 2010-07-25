// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTSTorus.h"
#include "GTSTorus.c7"

#include <Glasses/GTSIsoMaker.h>

// GTSTorus

//______________________________________________________________________________
//
//

ClassImp(GTSTorus);

//==============================================================================

void GTSTorus::_init()
{
  mRM = 1;
  mRm = 0.5;
  mStep = 0.01;
}

GTSTorus::GTSTorus(const Text_t* n, const Text_t* t) :
  GTSurf(n, t)
{
  _init();
}

GTSTorus::~GTSTorus()
{}

//==============================================================================

void GTSTorus::GTSIsoBegin(GTSIsoMaker* maker, Double_t /*iso_value*/)
{
  mRminvsqr = 1.0 / (mRm * mRm);

  Double_t RR = 1.05 * (mRM + mRm);
  Double_t rr = 1.05 * mRm;

  GLensReadHolder _lck(maker);
  maker->SetValue(1);
  maker->SetXAxis(-RR, RR, TMath::Nint(2.0*RR/mStep));
  maker->SetYAxis(-RR, RR, TMath::Nint(2.0*RR/mStep));
  maker->SetZAxis(-rr, rr, TMath::Nint(2.0*rr/mStep));
  maker->SetInvertCartesian(false);
  maker->SetInvertTetra(true);
}

Double_t GTSTorus::GTSIsoFunc(Double_t x, Double_t y, Double_t z)
{
  const Double_t r = TMath::Sqrt(x*x + y*y) - mRM;
  return mRminvsqr * (r*r + z*z);
}

void GTSTorus::GTSIsoEnd()
{

}
