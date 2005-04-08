// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_TubeTvor_H
#define GledCore_TubeTvor_H

#include <Gled/GledTypes.h>

struct RingInfo {
  Int_t		fIndex;
  Int_t		fNVert;
  Bool_t	fDrawP;

  RingInfo(Int_t i, Int_t n, Bool_t dp) : fIndex(i), fNVert(n), fDrawP(dp) {}
};

typedef vector<RingInfo>		vRingInfo_t;
typedef vector<RingInfo>::iterator	vRingInfo_i;


class TubeTvor {

public:

  Int_t		mPVI;
  Int_t		mNP;
  Int_t		mI;
  vRingInfo_t	mRings;
  Float_t*	mV;
  Float_t*	mN;
  UChar_t*	mC;
  Float_t*	mT;
  Bool_t	bColP;
  Bool_t	bTexP;

  //----------------------

  TubeTvor();
  ~TubeTvor();

  void Init(Int_t npoles, Int_t nrings, Int_t nphi,
	    Bool_t colp=false, Bool_t texp=false);
  void NewRing(Int_t n, Bool_t dp);
  void NewVert(Float_t *v, Float_t *n, UChar_t *c=0, Float_t* t=0);
}; // endclass GL_TubeTvor

inline void
TubeTvor::NewVert(Float_t *v, Float_t *n, UChar_t *c, Float_t *t) {
  {
  const int i = mI*3;
  mV[i] = v[0]; mV[i+1] = v[1]; mV[i+2] = v[2];
  mN[i] = n[0]; mN[i+1] = n[1]; mN[i+2] = n[2];
  }
  if(bColP) {
    const int i = mI*4;
    mC[i] = c[0]; mC[i+1] = c[1]; mC[i+2] = c[2]; mC[i+3] = c[3];
  }
  if(bTexP) {
    const int i = mI*2;
    mT[i] = t[0]; mT[i+1] = t[1];
  }
  ++mI;
}

#endif
