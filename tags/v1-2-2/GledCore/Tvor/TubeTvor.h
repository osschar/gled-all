// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_TubeTvor_H
#define Gled_TubeTvor_H

#include <Gled/GledTypes.h>

struct RingInfo {
  Int_t		fIndex;
  Int_t		fNVert;
  bool		fDrawP;
  //ZVector	mOrigin;	// -> v bistvu kar ZTrans ... or sth
  //ZVector	mAxis;		// skupaj s tem ...

  RingInfo(Int_t i, Int_t n, bool dp) : fIndex(i), fNVert(n), fDrawP(dp) {}
};

typedef vector<RingInfo>		vRingInfo_t;
typedef vector<RingInfo>::iterator	vRingInfo_i;

// od mojstra dobim ZVector, pa jih zbasem v array, katerega velikost
// poznam in si oznacim, pri katerem indexu se zacne ring.
// in morda se ... koliko je dolg ... in center in normalo ...

class TubeTvor {
  friend class TubeTvor_GL_Rnr;

private:
  Int_t		mPVI;
  Int_t		mNP;
  Int_t		mI;
  vRingInfo_t	mRings;
  float*	mV;
  float*	mN;
  float*	mC;
  float*	mT;
  bool		bColP;
  bool		bTexP;

public:
  TubeTvor();
  ~TubeTvor();

  void Init(Int_t npoles, Int_t nrings, Int_t nphi,
	    bool colp=false, bool texp=false);
  void NewRing(Int_t n, bool dp);
  void NewVert(float *v, float *n, float *c=0, float* t=0);
}; // endclass GL_TubeTvor

inline void
TubeTvor::NewVert(float *v, float *n, float *c, float *t) {
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
