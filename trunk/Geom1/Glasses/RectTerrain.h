// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_RectTerrain_H
#define Geom1_RectTerrain_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
class ZImage;

class RectTerrain : public ZNode {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(RectTerrain);

private:
  void _init();

protected:
  UInt_t	mNx;	// X{GS} 7 ValOut(-range=>[1,1000,1],-join=>1)
  UInt_t	mNy;	// X{GS} 7 ValOut(-range=>[1,1000,1])
  Real_t	mOx;	// X{GS} 7 Value(-range=>[-1e6,1e6,1,10],-join=>1)
  Real_t	mOy;	// X{GS} 7 Value(-range=>[-1e6,1e6,1,10])
  Real_t	mDx;	// X{GS} 7 Value(-range=>[0,10,1,100],-join=>1)
  Real_t	mDy;	// X{GS} 7 Value(-range=>[0,10,1,100])
  ZMatrix	mP;

  Float_t	mZScale;// X{GS}  7 Value(-range=>[0,1000,1,100])
  Float_t	mMinZ;	// X{GS}  7 ValOut(-join=>1)
  Float_t	mMaxZ;	// X{GS}  7 ValOut()
  ZColor	mMinCol;// X{PGS} 7 ColorButt(-join=>1)
  ZColor	mMaxCol;// X{PGS} 7 ColorButt()
  Real_t	mColSep;// X{GS}  7 Value(-range=>[0,100,1,10])

  Bool_t	bStudySize;	// X{GS} 7 Bool()

  void		MkVN(UCIndex_t i, UCIndex_t j);

public:
  RectTerrain(Text_t* n="RectTerrain", Text_t* t=0) : ZNode(n,t) { _init(); }

  void ToStd();		// X{E} 7 MButt(-join=>1)
  void ToCenter();	// X{E} 7 MButt()

  void SetFromImage(ZImage* image);	// X{E} C{1} 

#include "RectTerrain.h7"
  ClassDef(RectTerrain, 1)
}; // endclass RectTerrain

GlassIODef(RectTerrain);

#endif
