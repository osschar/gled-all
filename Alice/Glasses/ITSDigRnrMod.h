// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ITSDigRnrMod_H
#define Alice_ITSDigRnrMod_H

#include <Glasses/ZRnrModBase.h>
#include <Glasses/ZRibbon.h>
#include <Stones/ZColor.h>

class ITSDigRnrMod : public ZRnrModBase
{
  MAC_RNR_FRIENDS(ITSDigRnrMod);
  MAC_RNR_FRIENDS(ITSModule);

 private:
  void _init();

 protected:
  Short_t          mSDDTreshold;  // X{GST}  7 Value(-range=>[0,445,1], -join=>1)
  Int_t            mSDDMaxVal;    // X{GST}  7 Value(-range=>[0,100,1])
  Short_t          mSSDTreshold;  // X{GST}  7 Value(-range=>[0,445,1], -join=>1)
  Int_t            mSSDMaxVal;    // X{GST}  7 Value(-range=>[0,150,1])

  ZColor	   mMinCol;      // X{PGST} 7 ColorButt(-join=>1)
  ZColor           mMaxCol;      // X{PGST} 7 ColorButt()
  Float_t          mColSep;      // X{gST}  7 Value(-range=>[0,100,1,100])
  ZRibbon*         mRibbon;      // X{gST}  L{}

  Float_t          mDigitW;      // X{gST}  7 Value(-range=>[0,100,1,100], -join=>1)
  Float_t          mFrameW;      // X{gST}  7 Value(-range=>[0,100,1,100])
  Bool_t           bRnrFrame;	 // X{GST}  7 Bool()
  ZColor           mFrameCol;    // X{PGST} 7 ColorButt(-join=>1)

 public:
  ITSDigRnrMod(const Text_t* n="ITSDigRnrMod", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }

#include "ITSDigRnrMod.h7"
  ClassDef(ITSDigRnrMod, 1)
}; // endclass ITSDigRnrMod

GlassIODef(ITSDigRnrMod);

#endif
