// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TubeTvor.h"

TubeTvor::TubeTvor() : mV(0), mN(0), mC(0) {}

TubeTvor::~TubeTvor() {
  delete [] mV; delete [] mN; delete [] mC;
}

void TubeTvor::Init(bool colp, Int_t npoles, Int_t nrings, Int_t nphi)
{
  bColP = colp;
  mNP = npoles + nphi*nrings;
  mI = 0;
  mRings.clear();
  delete [] mV; delete [] mN; delete [] mC;
  mV = new float[mNP*3];
  mN = new float[mNP*3];
  if(bColP) mC = new float[mNP*4];
}

void TubeTvor::NewRing(Int_t n, bool dp)
{
  mRings.push_back( RingInfo(mI, n, dp) );
}
