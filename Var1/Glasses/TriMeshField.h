// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TriMeshField_H
#define Var1_TriMeshField_H

#include <Glasses/ZGlass.h>

class RGBAPalette;
class TriMesh;

class TriMeshField : public ZGlass
{
  MAC_RNR_FRIENDS(TriMeshField);

private:
  void _init();

protected:
  // Field: definition mesh, dimensions, values.

  ZLink<TriMesh>     mMesh;     // X{GS} L{A}
  Int_t              mNVerts;   // X{G} 7 ValOut(-join=>1)
  Int_t              mDim;      // X{G} 7 ValOut()
  vector<Float_t>    mField;    //


  // Color mapping of field values.

  ZLink<RGBAPalette> mPalette;  // X{GS} L{A}
  TString            mFormula;  // X{GS} 7 Textor()
  Float_t            mMinValue; // X{GS} 7 Value(-range=>[-1e6,1e6,1,100], -join=>1)
  Float_t            mMaxValue; // X{GS} 7 Value(-range=>[-1e6,1e6,1,100])


  // Var

  void check_min_max(Float_t v)
  { if (v < mMinValue) { mMinValue = v; } else { if (v > mMaxValue) mMaxValue = v; } }

public:
  TriMeshField(const Text_t* n="TriMeshField", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  Float_t* FVec(Int_t i=0)          { return &mField[i*mDim]; }
  Float_t& F   (Int_t i, Int_t j=0) { return  mField[i*mDim + j]; }
  Float_t& operator[](Int_t idx)    { return  mField[idx]; }

  void Resize(Int_t nvert, Int_t dim=0); // X{E} 7 MCWButt(-join=>1)
  void ResizeToMesh(Int_t dim=-1);       // X{E} 7 MCWButt()

  void SetField(Float_t c0);
  void SetField(Float_t c0, Float_t c1);
  void SetField(Float_t c0, Float_t c1, Float_t c2);

  void FindMinMaxField();  // X{E} 7 MCWButt()

  void ColorizeTvor(Bool_t regen_tring_cols=true);  // X{E} 7 MCWButt()
  void PartiallyColorizeTvor(set<Int_t> vertices, Bool_t regen_tring_cols=true);

  void FillByGaussBlobs(Int_t   n_blobs=40,
                        Float_t A_min=5,       Float_t A_max=20,
                        Float_t sigma_min=2,    Float_t sigma_max=10,
                        Bool_t  minmax_p=false, Bool_t  recolor_p=false); // X{E} 7 MCWButt()

  void Diffuse(Float_t diff_const=1, Float_t dt=0.1,
               Bool_t limit_df=false); // X{E} 7 MCWButt()

#include "TriMeshField.h7"
  ClassDef(TriMeshField, 1); // N-dimensional field spawning over tri-mesh vertices.
}; // endclass TriMeshField


#endif
