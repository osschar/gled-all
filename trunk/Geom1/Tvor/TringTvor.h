// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_TringTvor_H
#define Geom1_TringTvor_H

#include <Gled/GledTypes.h>

class TringTvor {
public:
  Bool_t   bSmoothShade;
  Bool_t   bColP;
  Bool_t   bTexP;

  // Vertex data

  Int_t    mNVerts;
  Float_t* mVerts;        //[3*mNVert]
  Float_t* mNorms;        //[3*mNVert]
  UChar_t* mCols;         //[3*mNVert]
  Float_t* mTexts;        //[2*mNVert]

  Float_t* Vertex(Int_t i)  { return &(mVerts[3*i]); }
  Float_t* Normal(Int_t i)  { return &(mNorms[3*i]); }
  UChar_t* Color(Int_t i)   { return &(mCols[3*i]);  }
  Float_t* Texture(Int_t i) { return &(mTexts[2*i]); }

  // Triangle data

  Int_t    mNTrings;
  Int_t*   mTrings;       //[3*mNTrings]
  Float_t* mTringNorms;   //[3*mNTrings]
  UChar_t* mTringCols;    //[3*mNVert]

  Int_t*   Triangle(Int_t i)       { return &(mTrings[3*i]);     }
  Float_t* TriangleNormal(Int_t i) { return &(mTringNorms[3*i]); }
  UChar_t* TriangleColor(Int_t i)  { return &(mTringCols[3*i]);  }

  // Triangle strip data

  Int_t    mNStripEls;
  Int_t*   mStripEls;     //[mNStripEls]
  Int_t*   mStripTrings;  //[mNStripEls]
  Int_t    mNStrips;
  Int_t**  mStripBegs;    //[mNStrips]
  Int_t*   mStripLens;    //[mNStrips]

  // --------------------------------------------------------------

  TringTvor(Int_t nv, Int_t nt, Bool_t smoothp,
	    Bool_t colp=false, Bool_t texp=false);
  ~TringTvor();

  void GenerateTriangleNormals();
  void GenerateTriangleNormalsAndColors(void (*foo)(Float_t*, UChar_t*, void*),
					void* ud);

  void GenerateTriangleStrips(Int_t max_verts=128);

}; // endclass TringTvor

#endif
