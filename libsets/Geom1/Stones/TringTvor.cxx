// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//==============================================================================
// TringTvor
//==============================================================================

//______________________________________________________________________
//
// Encapsulates low-level arrays for triangle meshes.
// Vertex and Triangle arrays are mandatory (called primary by methods).
// Optional:
//   per-vertex   normals and colors |
//   per-triangle normals and colors +-> secondary arrays
//   texture coords (per-vertex)     |
//   triangle strip data
//
// Serialization should mostly work. Secondary data is streamed as
// well.  As triangle-attributes (normals, colors) are usually
// calculated from vertex-attributes, some provision might be necessary to
// streamline this. Anyway ... this should really be managed by the lens
// using the tring-tvor as it also has the AdEnlightenment() virtual where
// post-serialization / pre-usage chores can be performed.
//
// Strip-data is not saved, nor is there any indication if it was used.

#include "TringTvor.h"
#include <TVector3.h>

#include <TMath.h>
#include <TRandom.h>

#define INV3                    0.33333333333333333333f

ClassImp(TringTvor);

void TringTvor::_init()
{
  mBBoxOK = false;

  mNStripEls   = 0;  mStripEls    = 0;  mStripTrings = 0;
  mNStrips     = 0;  mStripBegs   = 0;  mStripLens   = 0;
}

TringTvor::TringTvor() :
  mNVerts  (0),
  mNTrings (0)
{
  _init();
}

TringTvor::TringTvor(Int_t nv, Int_t nt) :
  mNVerts  (nv),
  mNTrings (nt)
{
  _init();
  MakePrimaryArrays();
}

TringTvor::TringTvor(Int_t nv, Int_t nt,
                     Bool_t smoothp, Bool_t colp, Bool_t texp) :
  mNVerts  (nv),
  mNTrings (nt)
{
  _init();
  MakePrimaryArrays();
  MakeSecondaryArrays(smoothp, colp, texp);
}

TringTvor::~TringTvor()
{
  DeleteTriangleStrips();
  DeleteSecondaryArrays();
  DeletePrimaryArrays();
}

void TringTvor::Reset(Int_t nv, Int_t nt)
{
  DeleteTriangleStrips();
  DeleteSecondaryArrays();
  DeletePrimaryArrays();
  mNVerts  = nv;
  mNTrings = nt;
  MakePrimaryArrays();
}

void TringTvor::Reset(Int_t nv, Int_t nt,
                      Bool_t smoothp, Bool_t colp, Bool_t texp)
{
  DeleteTriangleStrips();
  DeleteSecondaryArrays();
  DeletePrimaryArrays();
  mNVerts  = nv;
  mNTrings = nt;
  MakePrimaryArrays();
  MakeSecondaryArrays(smoothp, colp, texp);
}

/**************************************************************************/

void TringTvor::MakePrimaryArrays()
{
  mVerts .resize(3*mNVerts);
  mTrings.resize(3*mNTrings);
}

void TringTvor::DeletePrimaryArrays()
{
  WipeVerts();  mNVerts  = 0;
  WipeTrings(); mNTrings = 0;
}

void TringTvor::MakeSecondaryArrays(Bool_t smoothp, Bool_t colp, Bool_t texp)
{
  if (smoothp) {
    AssertNorms();
    if (colp) AssertCols();
  } else {
    AssertTringNorms();
    if (colp) AssertTringCols();
  }
  if (texp) AssertTexs();
}

void TringTvor::DeleteSecondaryArrays()
{
  WipeNorms();
  WipeCols();
  WipeTexs();

  WipeTringNorms();
  WipeTringCols();
}

/**************************************************************************/

Int_t TringTvor::AddVertices(Int_t nv)
{
  Int_t nold = mNVerts;
  mNVerts += nv;
  mVerts.resize(3*mNVerts);
  if (HasNorms()) MakeNorms();
  if (HasCols())  MakeCols();
  if (HasTexs())  MakeTexs();
  return nold;
}

Int_t TringTvor::AddTriangles(Int_t nt)
{
  Int_t nold = mNTrings;
  mNTrings += nt;
  mTrings.resize(3*mNTrings);
  if (HasTringNorms()) MakeTringNorms();
  if (HasTringCols())  MakeTringCols();
  return nold;
}

/**************************************************************************/

void TringTvor::CalculateBoundingBox()
{
  if (mNVerts == 0) {
    memset(mMinMaxBox, 0, 12*sizeof(Float_t));
    return;
  }

  Float_t *V = Verts(), *m = mMinMaxBox, *M = m + 3;
  m[0] = M[0] = V[0];
  m[1] = M[1] = V[1];
  m[2] = M[2] = V[2];
  V += 3;
  for (Int_t v=1; v<mNVerts; ++v) {
    if (V[0] < m[0]) m[0] = V[0]; else if (V[0] > M[0]) M[0] = V[0];
    if (V[1] < m[1]) m[1] = V[1]; else if (V[1] > M[1]) M[1] = V[1];
    if (V[2] < m[2]) m[2] = V[2]; else if (V[2] > M[2]) M[2] = V[2];
    V += 3;
  }

  Float_t *C = mCtrExtBox, *E = C + 3;
  C[0] = 0.5f*(M[0]+m[0]); C[1] = 0.5f*(M[1]+m[1]); C[2] = 0.5f*(M[2]+m[2]);
  E[0] = 0.5f*(M[0]-m[0]); E[1] = 0.5f*(M[1]-m[1]); E[2] = 0.5f*(M[2]-m[2]);

  mBBoxOK = true;
}

Float_t TringTvor::BoundingBoxDiagonal()
{
  Float_t *E = mCtrExtBox + 3;
  return 2.0f * sqrtf(E[0]*E[0] + E[1]*E[1] + E[2]*E[2]);
}

Float_t TringTvor::BoundingBoxXYArea()
{
  Float_t *E = mCtrExtBox + 3;
  return 4.0f * E[0]*E[1];
}

Float_t TringTvor::BoundingBoxVolume()
{
  Float_t *E = mCtrExtBox + 3;
  return 8.0f * E[0] * E[1] * E[2];
}

/**************************************************************************/

Float_t TringTvor::CalculateTriangleNormal(Int_t ti, Float_t normal[3])
{
  // Calculates triangle normal from vertex-data and stores it into 'normal'.
  // Returns original norm of the vector.
  //
  // This is to be used when triangle normals are not stored or during
  // construction of the tvor.

  const Int_t*    T = Triangle(ti);
  const Float_t* v0 = Vertex(T[0]);
  const Float_t* v1 = Vertex(T[1]);
  const Float_t* v2 = Vertex(T[2]);
  Float_t e1[3], e2[3];
  e1[0] = v1[0]-v0[0]; e1[1] = v1[1]-v0[1]; e1[2] = v1[2]-v0[2];
  e2[0] = v2[0]-v0[0]; e2[1] = v2[1]-v0[1]; e2[2] = v2[2]-v0[2];

  return TMath::NormCross(e1, e2, normal);
}

Float_t TringTvor::CalculateTriangleNormalAndCog(Int_t ti, Float_t normal[3], Float_t cog[3])
{
  // Calculates triangle normal from vertex-data and stores it into
  // 'normal'. Returns original norm of the vector. Center-of-gravity
  // of the triangle is returned in 'cog'.
  //
  // This is to be used when triangle normals are not stored or during
  // construction of the tvor.

  const Int_t*    T = Triangle(ti);
  const Float_t* v0 = Vertex(T[0]);
  const Float_t* v1 = Vertex(T[1]);
  const Float_t* v2 = Vertex(T[2]);
  Float_t e1[3], e2[3];
  e1[0] = v1[0]-v0[0]; e1[1] = v1[1]-v0[1]; e1[2] = v1[2]-v0[2];
  e2[0] = v2[0]-v0[0]; e2[1] = v2[1]-v0[1]; e2[2] = v2[2]-v0[2];

  cog[0] = (v0[0] + v1[0] + v2[0]) * INV3;
  cog[1] = (v0[1] + v1[1] + v2[1]) * INV3;
  cog[2] = (v0[2] + v1[2] + v2[2]) * INV3;

  return TMath::NormCross(e1, e2, normal);
}

void TringTvor::GenerateTriangleNormals()
{
  AssertTringNorms();

  for(Int_t t=0; t<mNTrings; ++t) 
  {
    CalculateTriangleNormal(t, TriangleNormal(t));
  }
}

void TringTvor::GenerateTriangleNormalsAndColors
  (void (*foo)(Float_t*, UChar_t*, void*), void* ud)
{
  AssertTringNorms();
  AssertTringCols();

  Float_t  cog[3];

  for(Int_t t=0; t<mNTrings; ++t)
  {
    CalculateTriangleNormalAndCog(t, TriangleNormal(t), cog);
    foo(cog, TriangleColor(t), ud);
  }
}

void TringTvor::GenerateTriangleColorsFromVertexColors()
{
  AssertTringCols();

  Int_t*    T = Trings();
  UChar_t*  C = TringCols();
  for(Int_t t=0; t<mNTrings; ++t, T+=3, C+=4) {
    UChar_t* c0 = Color(T[0]);
    UChar_t* c1 = Color(T[1]);
    UChar_t* c2 = Color(T[2]);
    C[0] = (UChar_t) (((UInt_t) c0[0] + c1[0] + c2[0]) * INV3);
    C[1] = (UChar_t) (((UInt_t) c0[1] + c1[1] + c2[1]) * INV3);
    C[2] = (UChar_t) (((UInt_t) c0[2] + c1[2] + c2[2]) * INV3);
    C[3] = (UChar_t) (((UInt_t) c0[3] + c1[3] + c2[3]) * INV3);
  }
}

void TringTvor::GenerateTriangleColorsFromVertexColors(set<Int_t>& triangles)
{
  AssertTringCols();

  for (set<Int_t>::iterator t = triangles.begin(); t != triangles.end(); ++t)
  {
    Int_t*    T = Triangle(*t);
    UChar_t*  C = TriangleColor(*t);
    UChar_t* c0 = Color(T[0]);
    UChar_t* c1 = Color(T[1]);
    UChar_t* c2 = Color(T[2]);
    C[0] = (UChar_t) (((UInt_t) c0[0] + c1[0] + c2[0]) * INV3);
    C[1] = (UChar_t) (((UInt_t) c0[1] + c1[1] + c2[1]) * INV3);
    C[2] = (UChar_t) (((UInt_t) c0[2] + c1[2] + c2[2]) * INV3);
    C[3] = (UChar_t) (((UInt_t) c0[3] + c1[3] + c2[3]) * INV3);
  }
}

void TringTvor::GenerateTriangleColorsFromTriangleStrips()
{
  // Assign random colors to different triangle strips.

  if (!HasTrianlgeStrips())
    return;
  AssertTringCols();

  TRandom rnd(0);

  for (Int_t i = 0; i < mNStrips; ++i)
  {
    UChar_t r = (UChar_t) rnd.Integer(256);
    UChar_t g = (UChar_t) rnd.Integer(256);
    UChar_t b = (UChar_t) rnd.Integer(256);

    Int_t* tring_idxp = mStripTrings + (mStripBegs[i] - mStripEls) + 2;
    Int_t  n          = mStripLens[i] - 2;
    while (n-- > 0)
    {
      UChar_t* tc = TriangleColor(*tring_idxp);
      tc[0] = r; tc[1] = g; tc[2] = b;
      ++tring_idxp;
    }
  }
}

/**************************************************************************/

void TringTvor::GenerateVertexNormals()
{
  // Generate per-vertex normals by averaging over normals of all triangles
  // sharing the vertex.
  // Requires building of intermediate structure.
  //
  // ??? Need an argument: weight normal contribution by triangle area
  // ??? or ... perhaps better ... by vertex angle.
  //     This is somewhat done in vertex-connection stuff in Var1::TriMesh.

  // Could reuse tring-normals if they do exist.

  vector<Int_t> *trings_per_vert = new vector<Int_t> [mNVerts];
  FindTrianglesPerVertex(trings_per_vert);

  AssertNorms();

  Float_t *v0, *v1, *v2;
  Int_t   *t;
  Float_t e1[3], e2[3], n[3];
  Float_t* N = &mNorms[0];
  for (Int_t i=0; i<mNVerts; ++i, N+=3) {
    vector<Int_t>& v = trings_per_vert[i];
    Int_t       size = (Int_t) v.size();
    //printf("V=%3d : Nt = %2d : ", i, size);
    N[0] = N[1] = N[2] = 0;
    for (Int_t j=0; j<size; ++j) {
      //printf("%3d ", v[j]);
      t  = Triangle(v[j]);
      v0 = Vertex(t[0]); v1 = Vertex(t[1]); v2 = Vertex(t[2]);
      e1[0] = v1[0] - v0[0]; e1[1] = v1[1] - v0[1]; e1[2] = v1[2] - v0[2];
      e2[0] = v2[0] - v0[0]; e2[1] = v2[1] - v0[1]; e2[2] = v2[2] - v0[2];
      TMath::NormCross(e1, e2, n);
      N[0] += n[0]; N[1] += n[1]; N[2] += n[2];
    }
    TMath::Normalize(N);
    //printf(" : %f %f %f\n", N[0], N[1], N[2]);
  }

  delete [] trings_per_vert;
}

/**************************************************************************/
// Intermediate structures
/**************************************************************************/

void TringTvor::FindTrianglesPerVertex(vector<Int_t>* trings_per_vert)
{
  // Populate array of vectors 'trings_per_vert' from triangle data.
  // The output array must be properly allocated in advance.

  Int_t* T = Trings();
  for (Int_t t=0; t<mNTrings; ++t, T+=3)
  {
    trings_per_vert[T[0]].push_back(t);
    trings_per_vert[T[1]].push_back(t);
    trings_per_vert[T[2]].push_back(t);
  }
}

Int_t TringTvor::FindNeighboursPerVertex(vector<Int_t>* neighbours)
{
  // Populate array of vectors 'neighbours' from triangle data.
  // The output array must be properly allocated in advance.
  // Returns total number of connections (which is 2*N_edge for closed
  // surfaces).

  const Int_t vP[3][2] = { { 0, 1 }, { 1, 2 }, { 2, 0 } };

  Int_t  nc = 0;
  Int_t* T  = Trings();
  for (Int_t t=0; t<mNTrings; ++t, T+=3)
  {
    for (Int_t eti=0; eti<3; ++eti)
    {
      const  Int_t   v0 = T[vP[eti][0]],   v1 = T[vP[eti][1]];
      vector<Int_t> &n0 = neighbours[v0], &n1 = neighbours[v1];

      if (find(n0.begin(), n0.end(), v1) == n0.end())
      { n0.push_back(v1); ++nc; }
      if (find(n1.begin(), n1.end(), v0) == n1.end())
      { n1.push_back(v0); ++nc; }
    }
  }
  return nc;
}


/**************************************************************************/
// Triangle strips
/**************************************************************************/

#include <ACTC/tc.h>

namespace {
  struct xx_tring {
    Int_t v1,v2,v3;
    xx_tring(Int_t _v1, Int_t _v2, Int_t _v3) : v1(_v1), v2(_v2), v3(_v3) {}

    bool operator==(const xx_tring& x) const
    // { return v1==x.v1 && v2==x.v2 && v3==x.v3;}
    { return
	(v1==x.v1 || v1==x.v2 || v1==x.v3) &&
	(v2==x.v1 || v2==x.v2 || v2==x.v3) &&
	(v3==x.v1 || v3==x.v2 || v3==x.v3);}
  };
}
namespace __gnu_cxx {
  template<>
  struct hash<xx_tring> {
    size_t operator()(const xx_tring& xx) const
    { size_t i = xx.v1 * xx.v2 * xx.v3; return i; }
  };
}

void TringTvor::GenerateTriangleStrips(Int_t max_verts)
{
  static const Exc_t _eh("TringTvor::GenerateTriangleStrips ");

  hash_map<xx_tring, Int_t> tring_map;

  ACTCData *tc = actcNew();
  if (tc == 0) throw(_eh + "failed to allocate TC structure.");
  // actcParami(tc, ACTC_OUT_MIN_FAN_VERTS, is maxint);
  // actcParami(tc, ACTC_OUT_MAX_PRIM_VERTS, 128);
  actcParami(tc, ACTC_OUT_MAX_PRIM_VERTS, max_verts);
  actcParami(tc, ACTC_OUT_HONOR_WINDING, 1);
  actcBeginInput(tc);
  for(Int_t t=0; t<mNTrings; ++t)
  {
    Int_t* T = Triangle(t);
    tring_map[ xx_tring(T[0], T[1], T[2]) ] = t;
    actcAddTriangle(tc, T[0], T[1], T[2]);
  }
  actcEndInput(tc);

  actcBeginOutput(tc);
  int prim;
  int v1, v2, v3;
  int cnt = 0, cntp;
  list<vector<int>* > strip_list;
  while ((prim = actcStartNextPrim(tc, &v1, &v2)) != ACTC_DATABASE_EMPTY)
  {
    cntp = 2;
    vector<int>* vecp = new vector<int>;
    vecp->push_back(v1); vecp->push_back(v2);
    while (actcGetNextVert(tc, &v3) != ACTC_PRIM_COMPLETE)
    {
      vecp->push_back(v3);
      ++cntp;
    }
    strip_list.push_back(vecp);
    cnt += cntp;
  }
  actcEndOutput(tc);

  actcDelete(tc);

  if (mNStripEls || mNStrips) DeleteTriangleStrips();

  mNStripEls = cnt;
  mNStrips   = strip_list.size();

  mStripEls    = new Int_t [mNStripEls];
  mStripTrings = new Int_t [mNStripEls];
  mStripBegs   = new Int_t*[mNStrips];
  mStripLens   = new Int_t [mNStrips];

  Int_t       idx = 0;
  Int_t strip_idx = 0;
  while (!strip_list.empty())
  {
    vector<int>* vecp = strip_list.front();
    Int_t s_len = vecp->size();
    mStripBegs[strip_idx] = &(mStripEls[idx]);
    mStripLens[strip_idx] = s_len;

    for (Int_t i=0; i<s_len; ++i, ++idx)
    {
      mStripEls[idx] = (*vecp)[i];

      if (i > 1) {
	xx_tring xx(mStripEls[idx-2], mStripEls[idx-1], mStripEls[idx]);
	hash_map<xx_tring, Int_t>::iterator xi;
	xi = tring_map.find(xx);
	if (xi != tring_map.end()) {
	  mStripTrings[idx] = xi->second;
	} else {
	  printf("%sSafr: %zu %d.\n", _eh.Data(), strip_list.size(), i);
	}
      }

    }

    strip_list.pop_front();
    delete vecp;
    ++strip_idx;
  }
}

void TringTvor::DeleteTriangleStrips()
{
  mNStripEls = 0;
  delete [] mStripEls;    mStripEls  = 0;
  delete [] mStripTrings; mStripTrings = 0;
  mNStrips = 0;
  delete [] mStripBegs;   mStripBegs = 0;
  delete [] mStripLens;   mStripLens = 0;
}

/**************************************************************************/
// Export
/**************************************************************************/

namespace {
template<typename T> void dump3vec(ostream& o, T v) {
  o << "< " << v[0] << ", " << v[1] << ", " << v[2] << " >, \n";
}
}

void TringTvor::ExportPovMesh(ostream& o, Bool_t smoothp)
{
  // Exports triangle-data as POV mesh2 object.
  // By experimentation I would say that smooth triangles do not work properly
  // in pov-3.6.1 (15.10.2006).

  o << "mesh2 {\n";

  {
    o << "  vertex_vectors { " << mNVerts << ",\n";
    Float_t* V = Verts();
    for (Int_t i=0; i<mNVerts; ++i, V+=3) {
      o << "    "; dump3vec<Float_t*>(o, V);
    }
    o << "  }\n";
  }

  if (smoothp) {
    Bool_t no_normals = ! HasNorms();
    if (no_normals) GenerateVertexNormals();
    o << "  normal_vectors { " << mNVerts << ",\n";
    Float_t* N = Norms();
    for (Int_t i=0; i<mNVerts; ++i, N+=3) {
      o << "    "; dump3vec<Float_t*>(o, N);
    }
    o << "  }\n";
    if (no_normals) { WipeNorms(); }
  }

  {
    o << "  face_indices { " << mNTrings << ",\n";
    Int_t* T = Trings();
    for (Int_t i=0; i<mNTrings; ++i, T+=3) {
      o << "    "; dump3vec<Int_t*>(o, T);
    }
    o << "  }\n";
  }

  o << "}\n";
}
