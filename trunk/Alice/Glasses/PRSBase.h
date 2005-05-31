// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_PRSBase_H
#define Alice_PRSBase_H

#include <Glasses/ZRnrModBase.h>
#include <Glasses/ZGlass.h>
#include <Stones/ZColor.h>
#include <Glasses/ZImage.h>

class PRSBase : public ZRnrModBase {
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(PRSBase);

 private:
  void _init();

 protected:

 public:
  Float_t                  mMagField;      // X{GSPT} 7 Value(-range=>[-10,10,1,100])
  //
  ZColor                   mVertexColor;   // X{GSPT} 7 ColorButt(-join=>1)
  Float_t                  mVertexSize;    // X{GSPT} 7 Value(-range=>[0.1,64,1,10])
  // track
  ZColor                   mTrackColor;    // X{GSPT} 7 ColorButt(-join=>1)
  Float_t                  mTrackWidth;    // X{GST}  7 Value(-range=>[0.1,128,1,10])
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
  // track textures
  ZImage*                  mTexture;      // X{gST} L{} RnrBits{4,0,5,0, 0,0,0,0}
  Float_t                  mTexFactor;    // X{GST}  7 Value(-range=>[1,1000,1])
  Float_t                  mTexVCoor;     // X{GST}  7 Value(-range=>[0.01,1,1,100])

  PRSBase(const Text_t* n="PRSBase", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }


#include "PRSBase.h7"
  ClassDef(PRSBase, 1)
    }; // endclass PRSBase

GlassIODef(PRSBase);

#endif
