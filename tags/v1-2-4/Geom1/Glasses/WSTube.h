// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_WSTube_H
#define Geom1_WSTube_H

#include <Glasses/WSSeed.h>
#include <TLorentzVector.h>
#include <TRandom.h>

class WSTube : public WSSeed {
  // 7777 RnrCtrl(1)
  MAC_RNR_FRIENDS(WSTube);

private:
  void _init();

protected:
  ZNode*	mNodeA;		// X{GS} L{}
  ZNode*	mNodeB;		// X{GS} L{}

  ZList*	mLenses;	// X{GS} L{} RnrBits{0,0,0,0, 0,0,0,5}

  Float_t	mDefWidth;	// X{GS} 7 Value(-range=>[  0,1000, 1,1000], join=>1)
  Float_t	mDefSpread;	// X{GS} 7 Value(-range=>[-180,180, 1,1000], join=>1)
  Float_t	mDefTension;	// X{GS} 7 Value(-range=>[-10,10,1,100])

  TLorentzVector mVecA;		// X{GSR} 7 LorentzVector()
  TLorentzVector mSgmA;		// X{GSR} 7 LorentzVector()
  TLorentzVector mVecB;		// X{GSR} 7 LorentzVector()
  TLorentzVector mSgmB;		// X{GSR} 7 LorentzVector()

  // SleepMS, InitDt used by AnimatedConnect and Travel.
  Int_t		mSleepMS;	// X{GS} 7 Value(-range=>[10,10000,1])
  Float_t	mInitDt;	// X{GS} 7 Value(-range=>[0,1,1,10000])

  TRandom	mRnd;

  void define_direction(ZTrans& t, TVector3& dr,
			TLorentzVector& vec, TLorentzVector& sgm);

public:
  WSTube(const Text_t* n="WSTube", const Text_t* t=0) :
    WSSeed(n,t) { _init(); }

  virtual void AdEnlightenment();
  virtual void AdEndarkenment();

  void Connect();         // X{E}  7 MButt(-join=>1)
  void AnimatedConnect(); // X{ED} 7 MButt()

  void TravelAtoB();                  // X{ED} 7 MButt(-join=>1)
  void TravelBtoA();                  // X{ED} 7 MButt()
  void Travel(Float_t abs_dt, UInt_t sleep_ms, Bool_t AtoB=true); // X{ED}

#include "WSTube.h7"
  ClassDef(WSTube, 1)
}; // endclass WSTube

GlassIODef(WSTube);

#endif