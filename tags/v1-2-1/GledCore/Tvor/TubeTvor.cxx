// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TubeTvor.h"

TubeTvor::TubeTvor() : mV(0), mN(0), mC(0), mT(0),
		       bColP(false), bTexP(false)
 {}

TubeTvor::~TubeTvor() {
  delete [] mV; delete [] mN; delete [] mC; delete [] mT;
}

void TubeTvor::Init(Int_t npoles, Int_t nrings, Int_t nphi,
		    bool colp, bool texp)
{
  delete [] mV; delete [] mN; delete [] mC; delete [] mT;
  bColP = colp;
  bTexP = texp;

  mNP = npoles + (nphi+1)*nrings;
  mI = 0;
  mRings.clear();
  mV = new float[mNP*3];
  mN = new float[mNP*3];
  if(bColP) mC = new float[mNP*4]; else mC = 0;
  if(bTexP) mT = new float[mNP*2]; else mT = 0;
}

void TubeTvor::NewRing(Int_t n, bool dp)
{
  mRings.push_back( RingInfo(mI, n, dp) );
}
