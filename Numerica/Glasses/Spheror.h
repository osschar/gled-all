// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Numerica_Spheror_H
#define Numerica_Spheror_H

#include <Glasses/ZNode.h>
#include <Glasses/WarmAmoeba.h>

#include <TVectorF.h>
#include <Stones/ZColor.h>

class Spheror : public ZNode, public WarmAmoebaMaster {
  // 7777 RnrCtrl(1)
  MAC_RNR_FRIENDS(Spheror);

private:
  void _init();

protected:
  UInt_t	mNVert;		// X{GS} 7 Value(-range=>[2,100,1,1], -join=>1)
private:
  UInt_t	mNV;		// X{G}  7 ValOut(-range=>[2,100,1,1])

protected:
  WarmAmoeba*	mAmoeba;	// X{GS} L{}

  TVectorF	mState;

  Bool_t	bBeautyP;	// X{GS}  7 Bool(-join=>1)
  ZColor	mColor;		// X{PGS} 7 ColorButt()
  Float_t	mSize;		// X{GS}  7 Value(-range=>[0,100,1,100], -join=>1)
  Float_t	mScale;		// X{GS}  7 Value(-range=>[0,100,1,10])

  TVectorF*	GimmeXYZ(const TVectorF& x);

public:
  Spheror(const Text_t* n="Spheror", const Text_t* t=0) : ZNode(n,t) {_init();}

  // virtuals
  virtual Float_t	Foon(const TVectorF& x);
  virtual TVectorF*	InitialState(TRandom& rnd);
  virtual TVectorF*	InitialPerturbations(TRandom& rnd);
  virtual void		SetState(const TVectorF& x); // X{E}

  void Install();	// X{E} C{0} 7 MButt(-join=>1)
  void SelfInit();	// X{E} C{0} 7 MButt()

#include "Spheror.h7"
  ClassDef(Spheror, 1)
}; // endclass Spheror

GlassIODef(Spheror);

#endif
