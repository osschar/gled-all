// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// MCTrackRnrStyle
//
//

#include "MCTrackRnrStyle.h"
#include "MCTrackRnrStyle.c7"



ClassImp(MCTrackRnrStyle);

/**************************************************************************/

void MCTrackRnrStyle::_init()
{
  // *** Set all links to 0 ***
  mFitDaughters = true;
  mFitDecay = true;

  mForceVisParents = false;
  mFixDaughterTime =true;
  mCheckT    = false;
  mMaxT      = 100.;
  mMinT      = 0;
  mFadeT     = 280;
  mKillT     = 300;
  mTScale    = -11.;
  mAnimDeltaT= 100;
  mUseSingleCol = true;
  mSingleCol.rgba(.6,1,.9,1.);
  mHeadCol.rgba(1,.2,.5,1.);
  mAlphaS = 0.2;
  mHeadS  = 0.2;

  mRnrPoints = false;

  // pdg color scheme 
  mDefCol.rgba(0.15,0.45,0.67,1);
  mECol.rgba(0.2,1,0.2,1);
  mMuCol.rgba(1,0.3,0.5,1);
  mGammaCol.rgba(1,1,0,1);
  mMesCol.rgba(0.5,0.5,1,1);
  mBarCol.rgba(1,0.8,1,1);
}

void MCTrackRnrStyle::calculate_abs_times()
{
  const Float_t scale = TMath::Power(10, mTScale);
  mMaxAT   = scale * mMaxT;
  mMinAT   = scale * mMinT;
  mFadeAT  = scale * mFadeT;
  mKillAT  = scale * mKillT; 
  mDeltaAT = mMaxAT - mMinAT;
  mAlphaAT = mMinAT + mDeltaAT * mAlphaS;
  mHeadAT  = mMaxAT - mDeltaAT * mHeadS;
}

/**************************************************************************/

ZColor MCTrackRnrStyle::GetPdgColor(Int_t pdg)
{
  Int_t pdga = TMath::Abs(pdg);
  ZColor col =  mDefCol;

  // elementary  particles
  if (pdga < 100) {
    switch (pdga) {
    case 11:  
      col = mECol; break; 
    case 12:
      col = mMuCol; break;
    case 22:
      col = mGammaCol; break;
    }
  }
  else if (pdga < 100000){ 
    Int_t i  = pdga;
    Int_t i0 = i%10; i /= 10;
    Int_t i1 = i%10; i /= 10; 
    Int_t i2 = i%10; i /= 10; 
    Int_t i3 = i%10; i /= 10; 
    Int_t i4 = i%10;
    // printf("pdg(%d) quark indices (%d,%d,%d,%d,%d) \n",pdg, i4,i3,i2, i1, i0);
    // meson
    if ((i3 == 0) && ( i4 < 2)){
      col = mMesCol; // quarks: i1,i2 (spin = i0)
      if(i1 == 3 || i2 == 3)
	col.g(1);
    } // barion
    else if ( i2 >= i1 && i3 >= i2 ) {
      col = mBarCol; // quarks: i1,i2, i3 (spin = i0))
    }
  }
  return col;
}

ZImage* MCTrackRnrStyle::GetPdgTexture(Int_t pdg)
{
  Int_t pdga = TMath::Abs(pdg);
  if (pdga > 100 && pdga < 100000) { 
    Int_t i  = pdga;
    Int_t i0 = i%10; i /= 10; // printf("i: %d \n", i);
    Int_t i1 = i%10; i /= 10; 
    Int_t i2 = i%10; i /= 10; 
    Int_t i3 = i%10; i /= 10; 
    Int_t i4 = i%10;
    // meson
    if ((i3 == 0) && (i4 < 2)) {
      if (i1 == 3 || i2 == 3)
	return *mTexture;          
    } // barion
    else if (i2 >= i1 && i3 >= i2) {
      if (i1 == 3 || i2 == 3 || i3 == 3)
	return *mTexture;
    }
  }
  return 0;
}

/**************************************************************************/

void MCTrackRnrStyle::TimeTick(Double_t t, Double_t dt)
{
  mMaxT = t;
  mMinT = mMaxT - mAnimDeltaT;
  mStampReqTring = Stamp(FID());
}
