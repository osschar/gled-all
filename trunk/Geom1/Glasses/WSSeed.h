// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_WSSeed_H
#define Geom1_WSSeed_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

#include <Glasses/ZImage.h>

class WSPoint;
class TubeTvor;

class WSSeed : public ZNode {

  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(WSSeed);

private:
  void _init();
  void InitSlide(WSPoint* f);
  void Ring(WSPoint* f, Float_t t);

  TVectorF	hPnt;	//! Coord-sys; used during Triangulation
  TVectorF	hAxe;	//!
  TVectorF	hUp;	//!
  TVectorF	hAw;	//!
  Float_t	hTexU;	//! Texture coords; used during Triangulation
  Float_t	hTexV;	//!

protected:
  Int_t		mTLevel;	// X{GST} 7 Value(-range=>[2,1000,1,1],-join=>1)
  Int_t		mPLevel;	// X{GST} 7 Value(-range=>[2,1000,1,1])

  Bool_t	bRenormLen;	// X{GST} 7 Bool(-join=>1)
  Float_t	mLength;	// X{GST} 7 Value(-range=>[0,1000,1,100])

  ZColor	mColor;		// X{PGS} 7 ColorButt(-join=>1)
  Int_t		mLineW;		// X{GST} 7 Value(-range=>[1,20,1,1],-join=>1)
  bool		bFat;		// X{GST} 7 Bool()

  ZImage*	mTexture;	// X{GST} L{} RnrBits{2,0,4,0, 0,0,0,0}

  TubeTvor*	pTuber;		//!
  Bool_t	bTextured;	//!

public:
  WSSeed(Text_t* n="WSSeed", Text_t* t=0) :
    ZNode(n,t), hPnt(5), hAxe(5), hUp(5), hAw(5) { _init(); }
  virtual ~WSSeed();

  // virtuals
  virtual void Triangulate();

#include "WSSeed.h7"
  ClassDef(WSSeed, 1)
}; // endclass WSSeed

GlassIODef(WSSeed);

#endif
