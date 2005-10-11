// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_MCTrackRnrStyle_H
#define Alice_MCTrackRnrStyle_H

#include <Glasses/PRSBase.h>
#include <Glasses/MCTrack.h>
#include <Glasses/ZImage.h>

#include <Stones/ZColor.h>
#include <Stones/TimeMakerClient.h>

class MCTrackRnrStyle : public PRSBase, public TimeMakerClient
{
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(MCTrackRnrStyle);
  MAC_RNR_FRIENDS(MCTrack);

 private:
  void _init();

  // protected:
 public:
  Bool_t   mFitDaughters; // X{GST} Stamp{PointCalcReq}  7 Bool(-join=>1)
  Bool_t   mFitDecay;     // X{GST} Stamp{PointCalcReq}  7 Bool()

  // helix limits 
  Bool_t                   mCheckT;          // X{GST}  7 Bool(-join=>1)
  Bool_t                   mFixDaughterTime; // X{GST}  7 Bool()
  Float_t                  mMaxT;            // X{GST}  7 Value(-range=>[0,10000, 1,100],-join=>1)
  Float_t                  mMinT;            // X{GST}  7 Value(-range=>[0,10000, 1,100])
  Float_t                  mMeanT;           // X{GE}   7 Value(-range=>[0,10000, 1,100],-join=>1)
  Float_t                  mDeltaT;          // X{GE}   7 Value(-range=>[0,10000, 1,100])
  Float_t                  mMaxTScale;       // X{GST}  7 Value(-range=>[-32,0, 1,100])
  Bool_t                   mForceVisParents; // X{GST} Stamp{PointCalcReq}  7 Bool(-join=>1)
  Bool_t                   mUseSingleCol;    // X{GST}  7 Bool(-join=>1)
  Bool_t                   mRnrPoints;       // X{GST}  7 Bool()
  ZColor                   mSingleCol;       // X{GSPT} 7 ColorButt(-join=>1)
  ZColor                   mHeadCol;         // X{GSPT} 7 ColorButt()
  Float_t                  mSatur;           // X{GST}  7 Value(-range=>[0,1, 1,1000])

  // PDG colors
  ZColor   mDefCol;       // X{GSPT} 7 ColorButt(-join=>1)
  ZColor   mECol;         // X{GSPT} 7 ColorButt()
  ZColor   mMuCol;        // X{GSPT} 7 ColorButt(-join=>1)
  ZColor   mGammaCol;     // X{GSPT} 7 ColorButt()
  ZColor   mMesCol;       // X{GSPT} 7 ColorButt(-join=>1)
  ZColor   mBarCol;       // X{GSPT} 7 ColorButt()

  // public:
  MCTrackRnrStyle(const Text_t* n="MCTrackRnrStyle", const Text_t* t=0) :
    PRSBase(n,t) { _init(); }
  ZColor                   GetPdgColor(Int_t pdg);
  ZImage*                  GetPdgTexture(Int_t pdg);

  // Manual Get/Set-methods
  void SetDeltaT(Float_t t){
    mDeltaT = t;
    mMaxT = mMeanT + mDeltaT/2; 
    mMinT = mMeanT - mDeltaT/2; 
    mStampReqTring = Stamp(FID());
  }

  void SetMeanT(Float_t t){
    mMeanT = t;
    mMaxT = mMeanT + mDeltaT/2; 
    mMinT = mMeanT - mDeltaT/2;
    mStampReqTring = Stamp(FID());
  }

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

#include "MCTrackRnrStyle.h7"
  ClassDef(MCTrackRnrStyle, 1);
}; // endclass MCTrackRnrStyle


#endif
