// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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
  void		Messofy(Float_t ct, Float_t st, Float_t phi);
  void		_init(Float_t r);

protected:
  Int_t		mTLevel;	// X{GST} 7 Value(-range=>[2,100, 1], -join=>1)
  Int_t		mPLevel;	// X{GST} 7 Value(-range=>[3,100, 1])

  Float_t	mTx;		// X{GST} 7 Value(-range=>[-10,10, 1,100], -join=>1)
  Float_t	mCx;		// X{GST} 7 Value(-range=>[-10,10, 1,100], -join=>1)
  Float_t	mRz;		// X{GST} 7 Value(-range=>[-10,10, 1,100])
  Bool_t	bOpenTop;	// X{GST} 7 Bool(-join=>1)
  Bool_t	bOpenBot;	// X{GST} 7 Bool()
  Bool_t	bEquiSurf;	// X{GST} 7 Bool()
  Bool_t	bUseDispList;	// X{GST} 7 Bool()
  ZColor	mColor;		// X{PGS} 7 ColorButt()

  ZImage*	mTexture;	// X{GST} L{} RnrBits{2,0,4,0, 0,0,0,0}

  TubeTvor*	pTuber;		//!
  Bool_t	bTextured;	//!

public:
  SMorph(const Text_t* n="SMorph", const Text_t* t=0) : ZNode(n,t)
  { _init(1); }
  SMorph(Float_t r, const Text_t* n="SMorph", const Text_t* t=0) : ZNode(n,t)
  { _init(r); }
  virtual ~SMorph();

  // virtuals
  virtual void Triangulate();

#include "SMorph.h7"
  ClassDef(SMorph, 1)
}; // endclass SMorph

GlassIODef(SMorph);

#endif
