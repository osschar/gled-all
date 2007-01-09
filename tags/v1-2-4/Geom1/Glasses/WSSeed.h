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

  // 7777 RnrCtrl(1)
  MAC_RNR_FRIENDS(WSSeed);

private:
  void _init();

  Float_t	hTexU;	//! Texture coords; used during Triangulation
  Float_t	hTexV;	//! and for storage of TexOrig for texture animation

protected:
  WSPoint* get_first_point();

  ZTrans* init_slide(WSPoint* f);
  void ring(ZTrans& lcf, WSPoint* f, Float_t t);

  Int_t		mTLevel;	// X{GST}  7 Value(-range=>[2,1000,1,1],-join=>1)
  Int_t		mPLevel;	// X{GST}  7 Value(-range=>[2,1000,1,1])

  Float_t	mTexUOffset;	// X{GE}   7 Value(-range=>[-1e6,1e6,1,1000], -join=>1)
  Float_t	mTexVOffset;	// X{GE}   7 Value(-range=>[-1e6,1e6,1,1000])

  Bool_t	bRenormLen;	// X{GST}  7 Bool(-join=>1)
  Float_t	mLength;	// X{GST}  7 Value(-range=>[0,1000,1,100])

  ZColor	mColor;		// X{PGST} 7 ColorButt(-join=>1)
  Float_t	mLineW;		// X{GST}  7 Value(-range=>[1,20,1,100],-join=>1)
  bool		bFat;		// X{GST}  7 Bool()

  ZImage*	mTexture;	//  X{GS}  L{} RnrBits{2,0,4,0, 0,0,0,0}

  TimeStamp_t	mStampReqTex;	//! X{GS}  Timestamp for re-texturing
  TimeStamp_t	mStampTexDone;	//! X{GS}  Timestamp when re-texturing done.

  TubeTvor*	pTuber;		//!
  Bool_t	bTextured;	//!

  Float_t	mDtexU;		// X{GS} 7 Value(-range=>[-1,1,1,1000], -join=>1)
  Float_t	mDtexV;		// X{GS} 7 Value(-range=>[-1,1,1,1000])
  Bool_t	bTexAnimOn;	// X{G } 7 BoolOut(-join=>1)
  UInt_t	mTASleepMS;	// X{GS} 7 Value(-range=>[10,10000,1])
public:
  void TexAnimStart();          // X{ED} 7 MButt(-join=>1)
  void TexAnimStop();           // X{E}  7 MButt()

protected:
  
  Bool_t	bUseDispList;	//! X{GS} 7 Bool()

public:
  WSSeed(const Text_t* n="WSSeed", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~WSSeed();

  void SetTexUOffset(Float_t texu);
  void SetTexVOffset(Float_t texv);

  // ZGlass virtuals
  virtual void SetStamps(TimeStamp_t s)
  { ZNode::SetStamps(s); mStampReqTex = s; mStampTexDone = s - 1; }

  // virtuals
  virtual void Triangulate();
  virtual void ReTexturize();

  void TransAtTime(ZTrans& lcf, Float_t time, bool repeat_p=false);

#include "WSSeed.h7"
  ClassDef(WSSeed, 1)
}; // endclass WSSeed

GlassIODef(WSSeed);

#endif