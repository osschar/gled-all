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
  void ring(ZTrans& lcf, WSPoint* f, Double_t t);

  Int_t		mTLevel;	// X{gST}  7 Value(-range=>[2,1000,1,1],-join=>1)
  Int_t		mPLevel;	// X{gST}  7 Value(-range=>[2,1000,1,1])

  Float_t	mTexUOffset;	// X{GS} Stamp{ReqTex} 7 Value(-range=>[-1e6,1e6,1,1000], -join=>1)
  Float_t	mTexVOffset;	// X{GS} Stamp{ReqTex} 7 Value(-range=>[-1e6,1e6,1,1000])

  Bool_t	bRenormLen;	// X{gST}  7 Bool(-join=>1)
  Float_t	mLength;	// X{gST}  7 Value(-range=>[0,1000,1,100])

  ZColor	mColor;		// X{PGST} 7 ColorButt(-join=>1)
  Float_t	mLineW;		// X{gST}  7 Value(-range=>[1,20,1,100],-join=>1)
  Bool_t	bFat;		// X{gST}  7 Bool()

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

  // ZGlass virtuals
  virtual void SetStamps(TimeStamp_t s)
  { ZNode::SetStamps(s); mStampReqTex = s; mStampTexDone = s - 1; }

  // virtuals
  virtual void Triangulate();
  virtual void ReTexturize();

  Float_t MeasureLength();
  void    MeasureAndSetLength(); // X{E} 7 MButt();

  void TransAtTime(ZTrans& lcf, Double_t time,
		   Bool_t repeat_p=false, Bool_t reinit_trans_p=false);

  void Travel(Double_t abs_dt=0.01, UInt_t sleep_ms=50,
	      Bool_t reverse_p=false); // X{ED} 7 MCWButt()

#include "WSSeed.h7"
  ClassDef(WSSeed, 1)
}; // endclass WSSeed


#endif
