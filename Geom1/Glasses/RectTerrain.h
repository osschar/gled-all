// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_RectTerrain_H
#define Geom1_RectTerrain_H

#include <Glasses/ZNode.h>
#include <Glasses/ZRibbon.h>
#include <Stones/ZColor.h>
class ZImage;
class TringTvor;

#include <TH2.h>

class RectTerrain : public ZNode
{
  MAC_RNR_FRIENDS(RectTerrain);

 public:
  enum RnrMode_e { RM_Histo, RM_SmoothSquare, RM_SmoothTring, RM_FlatTring };

  enum BorderCond_e { BC_Zero,  BC_Const, BC_External,
		      BC_Equal, BC_Sym, BC_ASym, BC_Wrap };

  enum OriginMode_e { OM_Edge, OM_Border, OM_Center };

 private:
  void _init();

 protected:
  Int_t		mNx;	// X{gS}  7 ValOut(-join=>1)
  Int_t		mNy;	// X{gS}  7 ValOut()
  Float_t	mDx;	// X{gST} 7 Value(-range=>[0,1000,1,1000],-join=>1)
  Float_t	mDy;	// X{gST} 7 Value(-range=>[0,1000,1,1000])
  TMatrixF	mP;

  Float_t	mMinZ;	 // X{gS} 7 ValOut(-join=>1)
  Float_t	mMaxZ;	 // X{gS} 7 ValOut()

  ZColor         mMinCol;  // X{PGST} 7 ColorButt(-join=>1)
  ZColor         mMaxCol;  // X{PGST} 7 ColorButt()
  Float_t        mColSep;  // X{gST}  7 Value(-range=>[0,100,1,1000])
  ZLink<ZRibbon> mRibbon;  // X{gST} L{}

  ZLink<ZImage>  mTexture; // X{gST} L{} RnrBits{4,0,5,0, 0,0,0,0}

  UChar_t       mRnrMode;     // X{gST} 7 PhonyEnum(-type=>RnrMode_e)
  UChar_t       mBorderCond;  // X{gS}  7 PhonyEnum(-type=>BorderCond_e)
  UChar_t       mOriginMode;  // X{gS}  7 PhonyEnum(-type=>OriginMode_e)
  Float_t	mBValue;      // X{gS}  7 Value(-range=>[-1000,1000,1,1000], -join=>1)
  Bool_t	bBorder;      // X{gST} 7 Bool()

  Float_t	mSmoothFac;   // X{gS}  7 Value(-range=>[0,1,1,1000])

  Bool_t	bStudySize;   // X{gS} 7 Bool()

  TringTvor*    pTTvor;          //! X{g}
  TimeStamp_t	mTTvorStamp;     //!
  Bool_t        bUseTringStrips; //  X{GST} 7 Bool(-join=>1)
  Int_t         mMaxTSVerts;     //  X{GST} 7 Value(-range=>[3,32767,1])

  ZColor make_color(Float_t z);
  static void color_filler(Float_t* v, UChar_t* c, void* rt);

 public:
  RectTerrain(Text_t* n="RectTerrain", Text_t* t=0) : ZNode(n,t) { _init(); }
  virtual ~RectTerrain();

  void ApplyBorderCondition(); // X{E} 7 MButt()

  void SetFromImage(ZImage* image, Float_t zfac=1); // X{E} C{1} 7 MCWButt()
  void SetFromHisto(TH2* histo);                    // X{E}

  void Smooth(Float_t fac=0); // X{E} 7 MButt(-join=>1)
  void RecalcMinMax();        // X{E} 7 MButt()

  void ReTring(); // X{E} 7 MButt(-join=>1)
  void Boobofy(); // X{E} 7 MButt()

  TringTvor* SpawnTringTvor(Bool_t smoothp, Bool_t flatp, Bool_t colp, Bool_t texp);

  void MakeTringTvor();

  Float_t GetMinX(Bool_t include_border=true) const;
  Float_t GetMaxX(Bool_t include_border=true) const;
  Float_t GetMinY(Bool_t include_border=true) const;
  Float_t GetMaxY(Bool_t include_border=true) const;

  static Float_t sMaxEpsilon;

#include "RectTerrain.h7"
  ClassDef(RectTerrain, 1);
}; // endclass RectTerrain


/**************************************************************************/

inline ZColor RectTerrain::make_color(Float_t z)
{
  if(mColSep > 0) {
    Float_t c = (z - mMinZ) * mColSep /	(mMaxZ - mMinZ);
    c -= (int)c;
    if(mRibbon != 0) {
      return mRibbon->MarkToCol(c);
    } else {
      return (1 - c)*mMinCol + c*mMaxCol;
    }
  } else {
    return mMinCol;
  }
}

#endif
