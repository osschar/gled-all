// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRlNameRnrCtrl_H
#define GledCore_ZRlNameRnrCtrl_H

#include <Glasses/ZGlStateBase.h>
#include <Stones/ZColor.h>

class ZRlNameRnrCtrl : public ZGlStateBase {
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZRlNameRnrCtrl);

private:
  void _init();

protected:
  ZGlStateBase::GlStateOp_e
                mNameRnrOp;   // X{GS}   7 PhonyEnum()

  Bool_t	bRnrTiles;    // X{GS}   7 Bool(-join=>1)
  Bool_t	bRnrFrames;   // X{GS}   7 Bool()
  Float_t	mNameOffset;  // X{GS}   7 Value(-range=>[0,1,1,10000])
  ZColor	mTextCol;     // X{PRGS} 7 ColorButt(-join=>1)
  ZColor	mTileCol;     // X{PRGS} 7 ColorButt()
  TString	mTilePos;     // X{RGS}  7 Textor()

public:
  ZRlNameRnrCtrl(const Text_t* n="ZRlNameRnrCtrl", const Text_t* t=0) :
    ZGlStateBase(n,t) { _init(); }

#include "ZRlNameRnrCtrl.h7"
  ClassDef(ZRlNameRnrCtrl, 1)
}; // endclass ZRlNameRnrCtrl

GlassIODef(ZRlNameRnrCtrl);

#endif
