// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliDet_TPCSegRnrMod_H
#define AliDet_TPCSegRnrMod_H


#include <Glasses/ZRnrModBase.h>
#include <Glasses/ZRibbon.h>
#include <Stones/ZColor.h>

class TPCSegRnrMod : public ZRnrModBase {
  MAC_RNR_FRIENDS(TPCSegRnrMod);
  MAC_RNR_FRIENDS(TPCSegment);

 private:
  void _init();

 protected:
  Bool_t		     bShowMax;	   // X{GST}  7 Bool(-join=>1)
  Bool_t		     bUseTexture;	   // X{GST}  7 Bool()
  Int_t                      mTime;        // X{GST}  7 Value(-range=>[0,445,1])
  Short_t                    mTreshold;    // X{GST}  7 Value(-range=>[0,445,1])
  Int_t                      mMaxVal;      // X{GST}  7 Value(-range=>[0,100,1])

  ZColor	             mMinCol;      // X{PGST} 7 ColorButt(-join=>1)
  ZColor		     mMaxCol;      // X{PGST} 7 ColorButt()
  Float_t	             mColSep;      // X{gST}  7 Value(-range=>[0,100,1,100])
  ZRibbon*	             mRibbon;      // X{gST}  L{}
  Bool_t		     bRnrFrame;	   // X{GST}  7 Bool()
  ZColor                     mFrameCol;    // X{PGST} 7 ColorButt(-join=>1)
  Float_t                    mAlpha;       // X{gST}  7 Value(-range=>[0,1,1,100])


 public:
  TPCSegRnrMod(const Text_t* n="TPCSegRnrMod", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }


#include "TPCSegRnrMod.h7"
  ClassDef(TPCSegRnrMod, 1)
    }; // endclass TPCSegRnrMod

GlassIODef(TPCSegRnrMod);
#endif