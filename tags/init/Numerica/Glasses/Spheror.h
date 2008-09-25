// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Numerica_Spheror_H
#define Numerica_Spheror_H

#include <Glasses/ZNode.h>
#include <Glasses/WarmAmoeba.h>

#include <Stones/ZVector.h>
#include <Stones/ZColor.h>

class Spheror : public ZNode, public WarmAmoebaMaster {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(Spheror);

private:
  void _init();

protected:
  UInt_t	mNVert;		// X{GS} 7 Value(-range=>[2,100,1,1], -join=>1)
private:
  UInt_t	mNV;		// X{G}  7 ValOut(-range=>[2,100,1,1])

protected:
  WarmAmoeba*	mAmoeba;	// X{GS} L{}

  ZVector	mState;

  Bool_t	bBeautyP;	// X{GS}  7 Bool(-join=>1)
  ZColor	mColor;		// X{PGS} 7 ColorButt()
  Real_t	mSize;		// X{GS}  7 Value(-range=>[0,100,1,100], -join=>1)
  Real_t	mScale;		// X{GS}  7 Value(-range=>[0,100,1,10])

  ZVector*	GimmeXYZ(const ZVector& x);

public:
  Spheror(const Text_t* n="Spheror", const Text_t* t=0) : ZNode(n,t) {_init();}

  // virtuals
  virtual Real_t	Foon(const ZVector& x);
  virtual ZVector*	InitialState(TRandom& rnd);
  virtual ZVector*	InitialPerturbations(TRandom& rnd);
  virtual void		SetState(const ZVector& x); // X{E}

  void Install();	// X{E} C{0} 7 MButt(-join=>1)
  void SelfInit();	// X{E} C{0} 7 MButt()

#include "Spheror.h7"
  ClassDef(Spheror, 1)
}; // endclass Spheror

GlassIODef(Spheror);

#endif
