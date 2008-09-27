// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_SMorph_H
#define Geom1_SMorph_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

class ZImage;
class TubeTvor;

class SMorph : public ZNode
{
  MAC_RNR_FRIENDS(SMorph);

private:
  void		Messofy(Float_t ct, Float_t st, Float_t phi);
  void		_init(Float_t r);

protected:
  Int_t		mTLevel;	// X{gST} 7 Value(-range=>[2,100, 1], -join=>1)
  Int_t		mPLevel;	// X{gST} 7 Value(-range=>[3,100, 1])

  Float_t	mTx;		// X{gST} 7 Value(-range=>[-10,10, 1,100], -join=>1)
  Float_t	mCx;		// X{gST} 7 Value(-range=>[-10,10, 1,100], -join=>1)
  Float_t	mRz;		// X{gST} 7 Value(-range=>[-10,10, 1,100])
  Bool_t	bOpenTop;	// X{gST} 7 Bool(-join=>1)
  Bool_t	bOpenBot;	// X{gST} 7 Bool()
  Bool_t	bEquiSurf;	// X{gST} 7 Bool()
  ZColor	mColor;		// X{PGST} 7 ColorButt()

  ZLink<ZImage>	mTexture;	// X{gST} L{} RnrBits{4,0,5,0}
  Float_t	mTexX0;         // X{gST} 7 Value(-range=>[-1e3,1e3,1,1000], -join=>1)
  Float_t	mTexY0;         // X{gST} 7 Value(-range=>[-1e3,1e3,1,1000])
  Float_t	mTexXC;         // X{gST} 7 Value(-range=>[-1e3,1e3,1,1000], -join=>1)
  Float_t	mTexYC;         // X{gST} 7 Value(-range=>[-1e3,1e3,1,1000])
  Float_t	mTexYOff;	// X{gST} 7 Value(-range=>[-10,10,1,1000])

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


#endif
