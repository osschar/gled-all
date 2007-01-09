// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_TringTvor_H
#define Geom1_TringTvor_H

#include <Gled/GledTypes.h>
#include <Stones/ZRCStone.h>

class TringTvor : public ZRCStone
{
private:
  void _init();

public:

  // Vertex data

  Int_t    mNVerts;
  Float_t* mVerts;        //[3*mNVert]
  Float_t* mNorms;        //[3*mNVert]
  UChar_t* mCols;         //[4*mNVert]
  Float_t* mTexs;         //[2*mNVert]

  Float_t  mMinMaxBox[6];
  Float_t  mCtrExtBox[6];

  void MakeNorms() { mNorms = new Float_t[3*mNVerts]; }
  void MakeCols()  { mCols  = new UChar_t[4*mNVerts]; }
  void MakeTexs()  { mTexs  = new Float_t[2*mNVerts]; }

  void AssertNorms() { if (!mNorms) MakeNorms(); }
  void AssertCols()  { if (!mCols)  MakeCols();  }
  void AssertTexs()  { if (!mTexs)  MakeTexs();  }

  Float_t* Vertex(Int_t i)  { return &(mVerts[3*i]); }
  Float_t* Normal(Int_t i)  { return &(mNorms[3*i]); }
  UChar_t* Color(Int_t i)   { return &(mCols[4*i]);  }
  Float_t* Texture(Int_t i) { return &(mTexs[2*i]); }

  // Triangle data

  Int_t    mNTrings;
  Int_t*   mTrings;       //[3*mNTrings]
  Float_t* mTringNorms;   //[3*mNTrings]
  UChar_t* mTringCols;    //[4*mNVert]

  void MakeTringNorms() { mTringNorms = new Float_t[3*mNTrings]; }
  void MakeTringCols()  { mTringCols  = new UChar_t[4*mNTrings]; }

  void AssertTringNorms() { if (!mTringNorms) MakeTringNorms(); }
  void AssertTringCols()  { if (!mTringCols)  MakeTringCols();  }

  Int_t*   Triangle(Int_t i)       { return &(mTrings[3*i]);     }
  Float_t* TriangleNormal(Int_t i) { return &(mTringNorms[3*i]); }
  UChar_t* TriangleColor(Int_t i)  { return &(mTringCols[4*i]);  }

  Bool_t   TriangleOtherVertices(Int_t t, Int_t v, Int_t& v_prev, Int_t& v_next);

  // Triangle strip data

  Int_t    mNStripEls;
  Int_t*   mStripEls;     //[mNStripEls]
  Int_t*   mStripTrings;  //[mNStripEls]
  Int_t    mNStrips;
  Int_t**  mStripBegs;    //[mNStrips]
  Int_t*   mStripLens;    //[mNStrips]

  // --------------------------------------------------------------

  TringTvor();
  TringTvor(Int_t nv, Int_t nt);
  TringTvor(Int_t nv, Int_t nt, Bool_t smoothp,
	    Bool_t colp=false, Bool_t texp=false);
  ~TringTvor();

  void MakePrimaryArrays();
  void DeletePrimaryArrays();
  void MakeSecondaryArrays(Bool_t smoothp, Bool_t colp=false, Bool_t texp=false);
  void DeleteSecondaryArrays();

  void SetVertex(Int_t i, Float_t x, Float_t y, Float_t z)
  { Float_t* v = Vertex(i); v[0] = x; v[1] = y; v[2] = z; }
  void SetNormal(Int_t i, Float_t x, Float_t y, Float_t z)
  { Float_t* v = Normal(i); v[0] = x; v[1] = y; v[2] = z; }
  void SetTriangle(Int_t i, Int_t v0, Int_t v1, Int_t v2)
  { Int_t* t = Triangle(i); t[0] = v0; t[1] = v1; t[2] = v2; }
  void SetTriangleColor(Int_t i, UChar_t r, UChar_t g, UChar_t b, UChar_t a=255)
  { UChar_t* c = TriangleColor(i); c[0] = r; c[1] = g; c[2] = b; c[3] = a; }

  void CalculateBoundingBox();

  void GenerateTriangleNormals();
  void GenerateTriangleNormalsAndColors(void (*foo)(Float_t*, UChar_t*, void*),
					void* ud);

  void GenerateVertexNormals();

  // Intermediate structures
  void FindTrianglesPerVertex(vector<Int_t>* trings_per_vert);
  void FindNeighboursPerVertex(vector<Int_t>* neighbours);

  // Triangle strips
  void GenerateTriangleStrips(Int_t max_verts=128);
  void DeleteTriangleStrips();

  // Export
  void ExportPovMesh(ostream& o, Bool_t smoothp=false);

  ClassDef(TringTvor, 0);
}; // endclass TringTvor


/**************************************************************************/
// Inlines
/**************************************************************************/

inline Bool_t TringTvor::TriangleOtherVertices(Int_t t, Int_t v, 
                                               Int_t& v_prev, Int_t& v_next)
{
  Int_t * T = Triangle(t);
  if (T[0] == v) { v_prev = T[2]; v_next = T[1]; return true; }
  if (T[1] == v) { v_prev = T[0]; v_next = T[2]; return true; }
  if (T[2] == v) { v_prev = T[1]; v_next = T[0]; return true; }
  return false;
}


#endif
