// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ZParticleRnrStyle_H
#define Alice_ZParticleRnrStyle_H

#include <Glasses/ZRnrModBase.h>
#include <Glasses/ZGlass.h>
#include <Stones/ZColor.h>
#include <Glasses/ZParticle.h>
#include <Glasses/ZImage.h>

class ZParticleRnrStyle : public ZRnrModBase {
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZParticleRnrStyle);
  MAC_RNR_FRIENDS(ZParticle);

 private:
  void _init();

  // protected:
 public:
  Bool_t                   mFitDaughters;  // X{GST}  7 Bool(-join=>1)
  Bool_t                   mFitDecay;      // X{GST}  7 Bool()
  ZColor                   mVertexColor;   // X{GSPT} 7 ColorButt(-join=>1)
  Float_t                  mVertexSize;    // X{GSPT} 7 Value(-range=>[0,50,1,10])
  // track
  // ZColor                   mTrackColor;    // X{GSPT} 7 ColorButt(-join=>1)
  Float_t                  mTrackWidth;    // X{GST}  7 Value(-range=>[0,10,1,10])
  // momentum
  ZColor                   mPColor;        // X{GSPT} 7 ColorButt(-join=>1)
  Bool_t                   mRnrP;          // X{GST}  7 Bool()
  Float_t                  mPMinLen;       // X{GST}  7 Value(-range=>[0,1000,1,10], -join=>1)
  Float_t                  mPScale;        // X{GST}  7 Value(-range=>[0.01,1,1,100])
  // track limits
  Float_t                  mMinP;          // X{GST}  7 Value(-range=>[0,10,  1,100000])
  Float_t                  mTheta;         // X{GST}  7 Value(-range=>[0,180, 1,10], -join=>1)
  Float_t                  mThetaOff;      // X{GST}  7 Value(-range=>[0,180, 1,10])
  Float_t                  mMaxR;          // X{GST}  7 Value(-range=>[0,100000,1], -join=>1)
  Float_t                  mMaxZ;          // X{GST}  7 Value(-range=>[0,100000,1])
  // helix limits 
  Int_t                    mMaxOrbs;       // X{GST}  7 Value(-range=>[0,500,1,100])
  Float_t                  mMinAng;        // X{GST}  7 Value(-range=>[1,180 ,1], -join=>1)
  Float_t                  mDelta;         // X{GST}  7 Value(-range=>[1e-3,100, 1,1000])

  // PDG colors
  ZColor   mDefCol;       // X{GSPT} 7 ColorButt(-join=>1)
  ZColor   mECol;         // X{GSPT} 7 ColorButt()
  ZColor   mMuCol;        // X{GSPT} 7 ColorButt(-join=>1)
  ZColor   mGammaCol;     // X{GSPT} 7 ColorButt()
  ZColor   mMesCol;       // X{GSPT} 7 ColorButt(-join=>1)
  ZColor   mBarCol;       // X{GSPT} 7 ColorButt()

  // track textures
  ZImage*                  mTexture;      // X{gST} L{} RnrBits{4,0,5,0, 0,0,0,0}
  Float_t                  mTexFactor;    // X{GST}  7 Value(-range=>[1,1000,1])
  Float_t                  mTexVCoor;     // X{GST}  7 Value(-range=>[0.01,1,1,100])

  // public:
  ZParticleRnrStyle(const Text_t* n="ZParticleRnrStyle", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }
  ZColor                   GetPdgColor(Int_t pdg);
  ZImage*                  GetPdgTexture(Int_t pdg);

#include "ZParticleRnrStyle.h7"
  ClassDef(ZParticleRnrStyle, 1)
    }; // endclass ZParticleRnrStyle

GlassIODef(ZParticleRnrStyle);

#endif
