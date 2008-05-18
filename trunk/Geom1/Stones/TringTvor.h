// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
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

  Int_t                mNVerts;
  std::vector<Float_t> mVerts;  // Vertex coordinates, 3*mNVert
  std::vector<Float_t> mNorms;  // Normals, 3*mNVert
  std::vector<UChar_t> mCols;   // RGBA color, 4*mNVert
  std::vector<Float_t> mTexs;   // Texture coordinates, 2*mNVert

  bool HasNorms() { return ! mNorms.empty(); }
  bool HasCols()  { return ! mCols .empty(); }
  bool HasTexs()  { return ! mTexs .empty(); }

  void MakeNorms() { mNorms.resize(3*mNVerts); }
  void MakeCols()  { mCols .resize(4*mNVerts); }
  void MakeTexs()  { mTexs .resize(2*mNVerts); }

  void AssertNorms() { if ( ! HasNorms()) MakeNorms(); }
  void AssertCols()  { if ( ! HasCols())  MakeCols();  }
  void AssertTexs()  { if ( ! HasTexs())  MakeTexs();  }

  void WipeVerts() { std::vector<Float_t> v; mVerts.swap(v); }
  void WipeNorms() { std::vector<Float_t> v; mNorms.swap(v); }
  void WipeCols()  { std::vector<UChar_t> v; mCols.swap(v);  }
  void WipeTexs()  { std::vector<Float_t> v; mTexs.swap(v);  }

  Float_t* Verts() { return &mVerts[0]; }
  Float_t* Norms() { return &mNorms[0]; }
  UChar_t* Cols()  { return &mCols[0]; }
  Float_t* Texs()  { return &mTexs[0]; }

  Float_t* Vertex(Int_t i)  { return &(mVerts[3*i]); }
  Float_t* Normal(Int_t i)  { return &(mNorms[3*i]); }
  UChar_t* Color(Int_t i)   { return &(mCols[4*i]);  }
  Float_t* Texture(Int_t i) { return &(mTexs[2*i]); }

  // Triangle data

  Int_t                mNTrings;
  std::vector<Int_t>   mTrings;       // Vertex-indices of triangles, 3*mNTrings
  std::vector<Float_t> mTringNorms;   // Triangle normals, 3*mNTrings
  std::vector<UChar_t> mTringCols;    // Triangle colors, 4*mNTrings

  Bool_t HasTringNorms()  { return ! mTringNorms.empty(); }
  Bool_t HasTringCols()   { return ! mTringCols .empty(); }

  void MakeTringNorms()   { mTringNorms.resize(3*mNTrings); }
  void MakeTringCols()    { mTringCols .resize(4*mNTrings); }

  void AssertTringNorms() { if ( ! HasTringNorms()) MakeTringNorms(); }
  void AssertTringCols()  { if ( ! HasTringCols())  MakeTringCols();  }

  void WipeTrings()     { std::vector<Int_t>   v; mTrings.swap(v);     }
  void WipeTringNorms() { std::vector<Float_t> v; mTringNorms.swap(v); }
  void WipeTringCols()  { std::vector<UChar_t> v; mTringCols.swap(v);  }

  Int_t*   Trings()     { return &mTrings[0];     }
  Float_t* TringNorms() { return &mTringNorms[0]; }
  UChar_t* TringCols()  { return &mTringCols[0];  }

  Int_t*   Triangle(Int_t i)       { return &mTrings[3*i];     }
  Float_t* TriangleNormal(Int_t i) { return &mTringNorms[3*i]; }
  UChar_t* TriangleColor(Int_t i)  { return &mTringCols[4*i];  }

  Bool_t   TriangleOtherVertices(Int_t t, Int_t v, Int_t& v_prev, Int_t& v_next);

  // Bounding-box stuff

  Bool_t   mBBoxOK;
  Float_t  mMinMaxBox[6];
  Float_t  mCtrExtBox[6];

  // Triangle strip data

  Int_t    mNStripEls;    //! Number of trianlge strip vertices (= 2*n-strips + n-of-triangles).
  Int_t*   mStripEls;     //! [mNStripEls] Vertex indices for all strips.
  Int_t*   mStripTrings;  //! [mNStripEls] Indices of triangles belonging to strip vertices, mNStripEls (first two triangles of each strip are not used). Needed for flat shading.
  Int_t    mNStrips;      //! Number of trianlge strips.
  Int_t**  mStripBegs;    //! [mNStrips] Pointers to strip beginnings into mStripEls, needed for glMultiDrawElements.
  Int_t*   mStripLens;    //! [mNStrips] Lengths of strips.  

  Bool_t   HasTrianlgeStrips() const { return mNStripEls != 0; }

  // --------------------------------------------------------------

  TringTvor();
  TringTvor(Int_t nv, Int_t nt);
  TringTvor(Int_t nv, Int_t nt,
            Bool_t smoothp, Bool_t colp=false, Bool_t texp=false);
  ~TringTvor();

  void Reset(Int_t nv, Int_t nt);
  void Reset(Int_t nv, Int_t nt,
             Bool_t smoothp, Bool_t colp=false, Bool_t texp=false);

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
  void AssertBoundingBox() { if (mBBoxOK == false) CalculateBoundingBox(); }

  Float_t BoundingBoxDiagonal();
  Float_t BoundingBoxXYArea();
  Float_t BoundingBoxVolume();

  void GenerateTriangleNormals();
  void GenerateTriangleNormalsAndColors(void (*foo)(Float_t*, UChar_t*, void*),
					void* ud);
  void GenerateTriangleColorsFromVertexColors();
  void GenerateTriangleColorsFromVertexColors(set<Int_t>& triangles);

  void GenerateTriangleColorsFromTriangleStrips();

  void GenerateVertexNormals();

  // Intermediate structures
  void  FindTrianglesPerVertex(vector<Int_t>* trings_per_vert);
  Int_t FindNeighboursPerVertex(vector<Int_t>* neighbours);

  // Triangle strips
  void GenerateTriangleStrips(Int_t max_verts=128);
  void DeleteTriangleStrips();

  // Export
  void ExportPovMesh(ostream& o, Bool_t smoothp=false);

  ClassDef(TringTvor, 1);
}; // endclass TringTvor


/**************************************************************************/
// Inlines
/**************************************************************************/

inline Bool_t TringTvor::TriangleOtherVertices(Int_t t, Int_t v,
                                               Int_t& v_prev, Int_t& v_next)
{
  // Find other two vertices (than v) in triangle t.
  // If you do: e1 = v_prev - v, e2 = v_next - v, then: e2 x e1 points
  // in the normal direction.

  Int_t * T = Triangle(t);
  if (T[0] == v) { v_prev = T[2]; v_next = T[1]; return true; }
  if (T[1] == v) { v_prev = T[0]; v_next = T[2]; return true; }
  if (T[2] == v) { v_prev = T[1]; v_next = T[0]; return true; }
  return false;
}


#endif
