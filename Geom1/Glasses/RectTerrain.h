// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_RectTerrain_H
#define Geom1_RectTerrain_H

#include <Glasses/ZNode.h>
#include <Glasses/ZRibbon.h>
#include <Stones/ZColor.h>
class ZImage;

#include <TH2.h>

class RectTerrain : public ZNode {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  MAC_RNR_FRIENDS(RectTerrain);

public:
  enum RnrMode_e { RM_Histo, RM_SmoothSquare };

  enum BorderCond_e { BC_Zero,  BC_Const, BC_External,
		      BC_Equal, BC_Sym, BC_ASym, BC_Wrap };

  enum OriginMode_e { OM_Edge, OM_Border, OM_Center };

private:
  void _init();

protected:
  Int_t		mNx;	// X{GS}  7 ValOut(-join=>1)
  Int_t		mNy;	// X{GS}  7 ValOut()
  Float_t	mDx;	// X{GST} 7 Value(-range=>[0,100,1,100],-join=>1)
  Float_t	mDy;	// X{GST} 7 Value(-range=>[0,100,1,100])
  TMatrixF	mP;

  Float_t	mMinZ;	// X{GS}   7 ValOut(-join=>1)
  Float_t	mMaxZ;	// X{GS}   7 ValOut()
  ZColor	mMinCol;// X{PGST} 7 ColorButt(-join=>1)
  ZColor	mMaxCol;// X{PGST} 7 ColorButt()
  Float_t	mColSep;// X{GST}  7 Value(-range=>[0,100,1,100])
  ZRibbon*	mRibbon;// X{GST} L{}

  UChar_t       mRnrMode;     // X{GST} 7 PhonyEnum(-type=>RnrMode_e)
  UChar_t       mBorderCond;  // X{GS}  7 PhonyEnum(-type=>BorderCond_e)
  UChar_t       mOriginMode;  // X{GS}  7 PhonyEnum(-type=>OriginMode_e)
  Float_t	mBValue;      // X{GS}  7 Value(-range=>[-100,100,1,1000], -join=>1)
  Bool_t	bBorder;      // X{GST} 7 Bool()

  Float_t	mSmoothFac;   // X{GS}  7 Value(-range=>[0,1,1,1000])

  Bool_t	bUseDispList;	// X{GS} 7 Bool(-join=>1)
  Bool_t	bStudySize;	// X{GS} 7 Bool()

  void		MkVN(Int_t i, Int_t j);

public:
  RectTerrain(Text_t* n="RectTerrain", Text_t* t=0) : ZNode(n,t) { _init(); }

  void ApplyBorderCondition();		// X{E} 7 MButt()

  void SetFromImage(ZImage* image);	// X{E} C{1} 
  void SetFromHisto(TH2* histo);	// X{E} 

  void Smooth(Float_t fac=0);		// X{E} 7 MButt(-join=>1)
  void RecalcMinMax();			// X{E} 7 MButt()

  void ReTring();			// X{E} 7 MButt(-join=>1)
  void Boobofy();			// X{E} 7 MButt()

  static Float_t sMaxEpsilon;

#include "RectTerrain.h7"
  ClassDef(RectTerrain, 1)
}; // endclass RectTerrain

GlassIODef(RectTerrain);

#endif
