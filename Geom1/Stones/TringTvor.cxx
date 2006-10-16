// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// TringTvor
//

#include "TringTvor.h"
#include <TVector3.h>

ClassImp(TringTvor);

void TringTvor::_init()
{
  mVerts  = 0;  mNorms      = 0;  mCols      = 0;  mTexs = 0;
  mTrings = 0;  mTringNorms = 0;  mTringCols = 0;

  mNStripEls   = 0;  mStripEls    = 0;  mStripTrings = 0;
  mNStrips     = 0;  mStripBegs   = 0;  mStripLens   = 0;
}

TringTvor::TringTvor() :
  bSmoothShade(false), bColP(false), bTexP(false),
  mNVerts  (0),
  mNTrings (0)
{
  _init();
}

TringTvor::TringTvor(Int_t nv, Int_t nt) :
  bSmoothShade(false), bColP(false), bTexP(false),
  mNVerts  (nv),
  mNTrings (nt)
{
  _init();
  MakePrimaryArrays();
}

TringTvor::TringTvor(Int_t nv, Int_t nt, Bool_t smoothp,
		     Bool_t colp, Bool_t texp) :
  bSmoothShade(smoothp), bColP(colp), bTexP(texp),
  mNVerts  (nv),
  mNTrings (nt)
{
  _init();
  MakePrimaryArrays();
  MakeSecondaryArrays();
}

TringTvor::~TringTvor()
{
  DeleteSecondaryArrays();
  DeleteTriangleStrips();
  DeletePrimaryArrays();
}

/**************************************************************************/

void TringTvor::MakePrimaryArrays()
{
  mVerts  = new Float_t[3*mNVerts];
  mTrings = new Int_t  [3*mNTrings];
}

void TringTvor::DeletePrimaryArrays()
{
  delete [] mVerts;  mVerts  = 0;
  delete [] mTrings; mTrings = 0;
}

void TringTvor::MakeSecondaryArrays()
{
  if(bSmoothShade) {
    mNorms = new Float_t[3*mNVerts];
    if(bColP) mCols = new UChar_t[4*mNVerts];
  } else {
    mTringNorms = new Float_t[3*mNTrings];
    if(bColP) mTringCols = new UChar_t[4*mNTrings];
  }
  if(bTexP) mTexs = new Float_t[2*mNVerts];
}

void TringTvor::DeleteSecondaryArrays()
{
  delete [] mNorms; mNorms = 0;
  delete [] mCols;  mCols  = 0;
  delete [] mTexs;  mTexs  = 0;

  delete [] mTringNorms; mTringNorms = 0;
  delete [] mTringCols;  mTringCols  = 0;
}

/**************************************************************************/

void TringTvor::GenerateTriangleNormals()
{
  if (!mTringNorms) MakeTringNorms();

  TVector3 e1, e2, n;
  for(Int_t t=0; t<mNTrings; ++t) {
    Int_t*    T = Triangle(t);
    Float_t* v0 = Vertex(T[0]);
    Float_t* v1 = Vertex(T[1]);
    Float_t* v2 = Vertex(T[2]);
    e1.SetXYZ(v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2]);
    e2.SetXYZ(v2[0]-v0[0], v2[1]-v0[1], v2[2]-v0[2]);
    n = e1.Cross(e2);
    n.SetMag(1);
    n.GetXYZ(TriangleNormal(t));
  }
}

void TringTvor::GenerateTriangleNormalsAndColors
  (void (*foo)(Float_t*, UChar_t*, void*), void* ud)
{
  if (!mTringNorms) MakeTringNorms();
  if (!mTringCols)  MakeTringCols();

  TVector3 e1, e2, n;
  Float_t  cg[3];

  for(Int_t t=0; t<mNTrings; ++t) {
    Int_t*    T = Triangle(t);
    Float_t* v0 = Vertex(T[0]);
    Float_t* v1 = Vertex(T[1]);
    Float_t* v2 = Vertex(T[2]);
    e1.SetXYZ(v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2]);
    e2.SetXYZ(v2[0]-v0[0], v2[1]-v0[1], v2[2]-v0[2]);
    n = e1.Cross(e2);
    n.SetMag(1);
    n.GetXYZ(TriangleNormal(t));

    cg[0] = (v0[0] + v1[0] + v2[0]) / 3;
    cg[1] = (v0[1] + v1[1] + v2[1]) / 3;
    cg[2] = (v0[2] + v1[2] + v2[2]) / 3;
    foo(cg, TriangleColor(t), ud);
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

  // Should generate tring-normals if they do not exist?
  // Depends ...

  vector<Int_t> *trings_per_vert = new vector<Int_t> [mNVerts];

  Int_t* T = mTrings;
  for (Int_t t=0; t<mNTrings; ++t, T+=3)
    {
      trings_per_vert[T[0]].push_back(t);
      trings_per_vert[T[1]].push_back(t);
      trings_per_vert[T[2]].push_back(t);
    }

  if (!mNorms) MakeNorms();

  Float_t *v0, *v1, *v2;
  Int_t   *t;
  Float_t e1[3], e2[3], n[3];
  Float_t* N = mNorms;
  for (Int_t i=0; i<mNVerts; ++i, N+=3)
    {
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
  if(tc == 0) throw(_eh + "failed to allocate TC structure.");
  // actcParami(tc, ACTC_OUT_MIN_FAN_VERTS, is maxint); // 
  // actcParami(tc, ACTC_OUT_MAX_PRIM_VERTS, 128);
  actcParami(tc, ACTC_OUT_MAX_PRIM_VERTS, max_verts);
  actcParami(tc, ACTC_OUT_HONOR_WINDING, 1);
  actcBeginInput(tc);
  for(Int_t t=0; t<mNTrings; ++t) {
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
  while((prim = actcStartNextPrim(tc, &v1, &v2)) != ACTC_DATABASE_EMPTY) {
    // printf("%s: %d %d", prim == ACTC_PRIM_FAN ? "Fan" : "Strip", v1, v2);
    cntp = 2;
    vector<int>* vecp = new vector<int>;
    vecp->push_back(v1); vecp->push_back(v2);
    while(actcGetNextVert(tc, &v3) != ACTC_PRIM_COMPLETE) {
      // printf(" %d", v3);
      vecp->push_back(v3);
      ++cntp;
    }
    // printf(" [%d]\n", cntp);
    strip_list.push_back(vecp);
    cnt += cntp;
  }
  // printf("### %d .vs. %d\n########\n", cnt, (maxX-minX)*(maxY-minY)*2*3);
  actcEndOutput(tc);

  actcDelete(tc);

  if (mNStripEls || mNStrips) DeleteTriangleStrips();

  mNStripEls = cnt;
  mNStrips   = strip_list.size();

  mStripEls    = new Int_t [mNStripEls];
  mStripTrings = new Int_t [mNStripEls];
  mStripBegs   = new Int_t*[mNStrips];
  mStripLens   = new Int_t [mNStrips];

  //printf("Now building strip data, num_idx=%d, num_strips=%d.\n",
  // mNStripEls, mNStrips);

  Int_t       idx = 0;
  Int_t strip_idx = 0;
  while(!strip_list.empty()) {
    vector<int>* vecp = strip_list.front();
    Int_t s_len = vecp->size();
    mStripBegs[strip_idx] = &(mStripEls[idx]);
    mStripLens[strip_idx] = s_len;

    for(Int_t i=0; i<s_len; ++i, ++idx) {
      mStripEls[idx] = (*vecp)[i];

      if(i > 1) {
	xx_tring xx(mStripEls[idx-2], mStripEls[idx-1], mStripEls[idx]);
	hash_map<xx_tring, Int_t>::iterator xi;
	xi = tring_map.find(xx);
	if(xi != tring_map.end()) {
	  mStripTrings[idx] = xi->second;
	} else {
	  printf("Safr: %d %d.\n", strip_list.size(), i);
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
  delete [] mStripEls;   mStripEls  = 0;
  delete [] mStripBegs;  mStripBegs = 0;
  delete [] mStripLens;  mStripLens = 0;
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
    Float_t* V = mVerts;
    for (Int_t i=0; i<mNVerts; ++i, V+=3) {
      o << "    "; dump3vec<Float_t*>(o, V);
    }
    o << "  }\n";
  }

  if (smoothp) {
    Bool_t no_normals = (mNorms == 0);
    if (no_normals) GenerateVertexNormals();
    o << "  normal_vectors { " << mNVerts << ",\n";
    Float_t* N = mNorms;
    for (Int_t i=0; i<mNVerts; ++i, N+=3) {
      o << "    "; dump3vec<Float_t*>(o, N);
    }
    o << "  }\n";
    if (no_normals) { delete [] mNorms; mNorms = 0; }
  }

  {
    o << "  face_indices { " << mNTrings << ",\n";
    Int_t* T = mTrings;
    for (Int_t i=0; i<mNTrings; ++i, T+=3) {
      o << "    "; dump3vec<Int_t*>(o, T);
    }
    o << "  }\n";
  }  

  o << "}\n";
}
