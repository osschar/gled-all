// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Text_H
#define GledCore_Text_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

class Text : public ZNode {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(Text);

private:
  void _init();

protected:
  TString	mText;		// X{GST}  7 Textor()
  Bool_t	bAlpha;		// X{GST}  7 Bool(-join=>1)
  Bool_t	bBlend;		// X{GST}  7 Bool()
  Bool_t	bPolyOffFG;	// X{GST}  7 Bool(-join=>1)
  Bool_t	bPolyOffBG;	// X{GST}  7 Bool()
  Float_t	mPOFac;		// X{GST}  7 Value(-range=>[-10,10,1,100], -join=>1)
  Float_t	mPOUni;		// X{GST}  7 Value(-range=>[-10,10,1,100])
  Bool_t	bAbsSize;	// X{GST}  7 Bool(-join=>1)
  Bool_t	bFillBack;	// X{GST}  7 Bool(-join=>1)
  Bool_t	bBorder;	// X{GST}  7 Bool()
  Float_t	mXBorder;	// X{GST}  7 Value(-range=>[0,100,1,100], -join=>1)
  Float_t	mYBorder;	// X{GST}  7 Value(-range=>[0,100,1,100])
  ZColor	mFGCol;		// X{PGST} 7 ColorButt(-join=>1)
  ZColor	mBGCol;		// X{PGST} 7 ColorButt()

public:
  Text(const Text_t* n="Text", const Text_t* t=0) : ZNode(n,t) { _init(); }


#include "Text.h7"
  ClassDef(Text, 1)
}; // endclass Text

GlassIODef(Text);

#endif
