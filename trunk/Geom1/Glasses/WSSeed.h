// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_WSSeed_H
#define Geom1_WSSeed_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

#include <Glasses/ZImage.h>

class WSPoint;
class TubeTvor;

class WSSeed : public ZNode
{
  MAC_RNR_FRIENDS(WSSeed);

private:
  void _init();

  Float_t	hTexU;	//! Texture coords; used during Triangulation
  Float_t	hTexV;	//! and for storage of TexOrig for texture animation

protected:
  WSPoint* get_first_point();

  ZTrans* init_slide(WSPoint* f);
  void ring(ZTrans& lcf, WSPoint* f, Float_t t);

  Int_t		mTLevel;	// X{gST}  7 Value(-range=>[2,1000,1,1],-join=>1)
  Int_t		mPLevel;	// X{gST}  7 Value(-range=>[2,1000,1,1])

  Float_t	mTexUOffset;	// X{gE}   7 Value(-range=>[-1e6,1e6,1,1000], -join=>1)
  Float_t	mTexVOffset;	// X{gE}   7 Value(-range=>[-1e6,1e6,1,1000])

  Bool_t	bRenormLen;	// X{gST}  7 Bool(-join=>1)
  Float_t	mLength;	// X{gST}  7 Value(-range=>[0,1000,1,100])

  ZColor	mColor;		// X{PGST} 7 ColorButt(-join=>1)
  Float_t	mLineW;		// X{gST}  7 Value(-range=>[1,20,1,100],-join=>1)
  bool		bFat;		// X{gST}  7 Bool()

  ZLink<ZImage>	mTexture;	// X{gS} L{} RnrBits{4,0,5,0, 0,0,0,0}

  TimeStamp_t	mStampReqTex;	//! X{gS}  Timestamp for re-texturing
  TimeStamp_t	mStampTexDone;	//! X{gS}  Timestamp when re-texturing done.

  TubeTvor*	pTuber;		//!
  Bool_t	bTextured;	//!

  Float_t	mDtexU;		// X{gS} 7 Value(-range=>[-1,1,1,1000], -join=>1)
  Float_t	mDtexV;		// X{gS} 7 Value(-range=>[-1,1,1,1000])
  Bool_t	bTexAnimOn;	// X{g } 7 BoolOut(-join=>1)
  UInt_t	mTASleepMS;	// X{gS} 7 Value(-range=>[10,10000,1])
public:
  void TexAnimStart();          // X{ED} 7 MButt(-join=>1)
  void TexAnimStop();           // X{E}  7 MButt()


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


#endif
