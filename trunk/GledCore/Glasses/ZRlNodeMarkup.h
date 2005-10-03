// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRlNodeMarkup_H
#define GledCore_ZRlNodeMarkup_H

#include <Glasses/ZRnrModBase.h>
#include <Stones/ZColor.h>

class ZRlNodeMarkup : public ZRnrModBase {
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZRlNodeMarkup);

private:
  void _init();

protected:
  ZRnrModBase::Operation_e mNodeMarkupOp; // X{GS}   7 PhonyEnum()

  Bool_t        bRnrAxes;     // X{GS}   7 Bool()
  Float_t       mAxeWidth;    // X{GS}   7 Value(-range=>[0,128,1,100], -join=>1)
  Float_t       mAxeLength;   // X{GS}   7 Value(-range=>[-100,100,1,100])
  Bool_t	bRnrNames;    // X{GS}   7 Bool()
  Bool_t	bRnrTiles;    // X{GS}   7 Bool(-join=>1)
  Bool_t	bRnrFrames;   // X{GS}   7 Bool()
  Float_t	mNameOffset;  // X{GS}   7 Value(-range=>[-2,2,1,10000])
  ZColor	mTextCol;     // X{PRGS} 7 ColorButt(-join=>1)
  ZColor	mTileCol;     // X{PRGS} 7 ColorButt()
  TString	mTilePos;     // X{RGS}  7 Textor()

public:
  ZRlNodeMarkup(const Text_t* n="ZRlNodeMarkup", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }

#include "ZRlNodeMarkup.h7"
  ClassDef(ZRlNodeMarkup, 1)
}; // endclass ZRlNodeMarkup


#endif
