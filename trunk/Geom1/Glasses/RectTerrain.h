// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_RectTerrain_H
#define Geom1_RectTerrain_H

#include <Glasses/ZNode.h>
#include <Glasses/ZRibbon.h>
#include <Stones/ZColor.h>
class ZImage;

#include <TH2.h>

/**************************************************************************/

class RectTringTvor {
public:
  Int_t          mNVerts;
  Float_t*       mVerts;        //[3*mNVert]
  Float_t*       mNorms;        //[3*mNVert]
  UChar_t*       mCols;         //[3*mNVert]

  Float_t* Vertex(Int_t i) { return &(mVerts[3*i]); }
  Float_t* Normal(Int_t i) { return &(mNorms[3*i]); }
  UChar_t* Color(Int_t i)  { return &(mCols[3*i]);  }

  Int_t          mNTrings;
  Int_t*         mTrings;       //[3*mNTrings]
  Float_t*       mTringNorms;   //[3*mNTrings]

  Int_t*   Triangle(Int_t i)       { return &(mTrings[3*i]); }
  Float_t* TriangleNormal(Int_t i) { return &(mTringNorms[3*i]); }

  Int_t          mNStripEls;
  Int_t*         mStripEls;     //[mNStripEls]
  Int_t*         mStripTrings;  //[mNStripEls]
  Int_t          mNStrips;
  Int_t**        mStripBegs;    //[mNStrips]
  Int_t*         mStripLens;    //[mNStrips]

  RectTringTvor(Int_t nv, Int_t nt) {
    mNVerts = nv;
    mVerts = new Float_t[3*nv];
    mNorms = new Float_t[3*nv];
    mCols  = new UChar_t[3*nv];
    mNTrings = nt;
    mTrings     = new Int_t[3*nt];
    mTringNorms = new Float_t[3*nt];

    mStripEls    = 0;
    mStripTrings = 0;
    mStripBegs = 0;
    mStripLens = 0;
  }
  ~RectTringTvor()
  {
    delete [] mVerts;  delete [] mNorms; delete [] mCols;
    delete [] mTrings; delete [] mTringNorms;
    delete [] mStripEls; delete [] mStripBegs; delete [] mStripLens;
  }

  void GenerateTriangleNormals();
  void GenerateTriangleStrips();
};

/**************************************************************************/

class RectTerrain : public ZNode {
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
  Float_t	mDx;	// X{gST} 7 Value(-range=>[0,100,1,100],-join=>1)
  Float_t	mDy;	// X{gST} 7 Value(-range=>[0,100,1,100])
  TMatrixF	mP;

  Float_t	mMinZ;	// X{gS}   7 ValOut(-join=>1)
  Float_t	mMaxZ;	// X{gS}   7 ValOut()
  ZColor	mMinCol;// X{PGST} 7 ColorButt(-join=>1)
  ZColor	mMaxCol;// X{PGST} 7 ColorButt()
  Float_t	mColSep;// X{gST}  7 Value(-range=>[0,100,1,100])
  ZRibbon*	mRibbon;// X{gST} L{}

  UChar_t       mRnrMode;     // X{gST} 7 PhonyEnum(-type=>RnrMode_e)
  UChar_t       mBorderCond;  // X{gS}  7 PhonyEnum(-type=>BorderCond_e)
  UChar_t       mOriginMode;  // X{gS}  7 PhonyEnum(-type=>OriginMode_e)
  Float_t	mBValue;      // X{gS}  7 Value(-range=>[-100,100,1,1000], -join=>1)
  Bool_t	bBorder;      // X{gST} 7 Bool()

  Float_t	mSmoothFac;   // X{gS}  7 Value(-range=>[0,1,1,1000])

  Bool_t	bStudySize;	// X{gS} 7 Bool()

  RectTringTvor* pRTTvor;          //! X{g}
  Bool_t         bUseTringStrips;  //  X{GST} 7 Bool()

  void		MkVN(Int_t i, Int_t j);

public:
  RectTerrain(Text_t* n="RectTerrain", Text_t* t=0) : ZNode(n,t) { _init(); }
  virtual ~RectTerrain();

  void ApplyBorderCondition();		// X{E} 7 MButt()

  void SetFromImage(ZImage* image);	// X{E} C{1} 7 MCWButt()
  void SetFromHisto(TH2* histo);	// X{E} 

  void Smooth(Float_t fac=0);		// X{E} 7 MButt(-join=>1)
  void RecalcMinMax();			// X{E} 7 MButt()

  void ReTring();			// X{E} 7 MButt(-join=>1)
  void Boobofy();			// X{E} 7 MButt()

  void Tringoo();

  static Float_t sMaxEpsilon;

#include "RectTerrain.h7"
  ClassDef(RectTerrain, 1)
}; // endclass RectTerrain

GlassIODef(RectTerrain);

#endif
