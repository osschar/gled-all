// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_SMorph_H
#define Geom1_SMorph_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

#include <Glasses/ZImage.h>

class TubeTvor;

class SMorph : public ZNode {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(SMorph);

private:
  void		Messofy(Real_t ct, Real_t st, Real_t phi);
  void		_init(Real_t r);

protected:
  Int_t		mTLevel;	// X{GST} 7 Value(-range=>[2,100, 1], -join=>1)
  Int_t		mPLevel;	// X{GST} 7 Value(-range=>[3,100, 1])
  Real_t	mSx;		// X{GS}  7 Value(-range=>[0,100, 1,100], -join=>1)
  Real_t	mSy;		// X{GS}  7 Value(-range=>[0,100, 1,100], -join=>1)
  Real_t	mSz;		// X{GS}  7 Value(-range=>[0,100, 1,100])
  Real_t	mTx;		// X{GST} 7 Value(-range=>[-10,10, 1,100], -join=>1)
  Real_t	mCx;		// X{GST} 7 Value(-range=>[-10,10, 1,100], -join=>1)
  Real_t	mRz;		// X{GST} 7 Value(-range=>[-10,10, 1,100])
  Bool_t	bOpenTop;	// X{GST} 7 Bool(-join=>1)
  Bool_t	bOpenBot;	// X{GST} 7 Bool()
  Bool_t	bNormalize;	// X{GS}  7 Bool(-join=>1)
  Bool_t	bEquiSurf;	// X{GST} 7 Bool()
  ZColor	mColor;		// X{PGS} 7 ColorButt()

  ZImage*	mTexture;	// X{GST} L{} RnrBits{2,0,4,0, 0,0,0,0}

  TubeTvor*	pTuber;		//!
  Bool_t	bTextured;	//!

public:
  SMorph(const Text_t* n="SMorph", const Text_t* t=0) : ZNode(n,t)
  { _init(1); }
  SMorph(Real_t r, const Text_t* n="SMorph", const Text_t* t=0) : ZNode(n,t)
  { _init(r); }
  virtual ~SMorph();

  void SetS(Real_t xx) { mSx = mSy = mSz = xx; Stamp(LibID(), ClassID()); } // X{e}
  void MultS(Real_t s) { mSx*=s; mSy*=s; mSz*=s; Stamp(LibID(), ClassID()); }
  void SetSs(Real_t x, Real_t y, Real_t z) { mSx=x; mSy=y; mSz=z; Stamp(LibID(), ClassID()); }

  // virtuals
  virtual void Triangulate();

#include "SMorph.h7"
  ClassDef(SMorph, 1)
}; // endclass SMorph

GlassIODef(SMorph);

#endif
