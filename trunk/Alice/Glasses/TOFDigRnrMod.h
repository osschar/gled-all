// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_TOFDigRnrMod_H
#define Alice_TOFDigRnrMod_H

#include <Glasses/ZRnrModBase.h>
#include <Stones/ZColor.h>
#include <Glasses/ZRibbon.h>

class TOFDigRnrMod : public ZRnrModBase {
  MAC_RNR_FRIENDS(TOFDigRnrMod);
  MAC_RNR_FRIENDS(TOFSector);
 private:
  void _init();

 protected: 
  Bool_t           bRnrModFrame;	 // X{GST}  7 Bool()
  Bool_t           bRnrStripFrame;	 // X{GST}  7 Bool(-join=>1)
  Bool_t           bRnrEmptyStrip;       // X{GST}  7 Bool()
  Bool_t           bRnrFlat;             // X{GST}  7 Bool()
  ZColor           mStripCol;            // X{PGST} 7 ColorButt(-join=>1)
  ZColor           mPlateCol;            // X{PGST} 7 ColorButt()

  ZColor	   mMinCol;      // X{PGST} 7 ColorButt(-join=>1)
  ZColor           mMaxCol;      // X{PGST} 7 ColorButt()
  Float_t          mColSep;      // X{gST}  7 Value(-range=>[0,100,1,100])
  Float_t          mTdcScale;    // X{gST}  7 Value(-range=>[0,10,1,1000])
  Float_t          mTdcMinH;     // X{gST}  7 Value(-range=>[0,100,1,100])
  ZRibbon*         mRibbon;      // X{gST}  L{}

 public:
  TOFDigRnrMod(const Text_t* n="TOFDigRnrMod", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }

 
#include "TOFDigRnrMod.h7"
  ClassDef(TOFDigRnrMod, 1)
    }; // endclass TOFDigRnrMod

GlassIODef(TOFDigRnrMod);

#endif
