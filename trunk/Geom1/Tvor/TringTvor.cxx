// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// TringTvor
//

#include "TringTvor.h"
#include <TVector3.h>

TringTvor::TringTvor(Int_t nv, Int_t nt, Bool_t smoothp,
		     Bool_t colp, Bool_t texp) :
  bSmoothShade(smoothp),
  bColP(colp), bTexP(texp),
  mNVerts(nv),
  mNTrings(nt)
{
  mVerts  = new Float_t[3*mNVerts];  mNorms = 0; mCols = 0; mTexs = 0;
  mTrings = new Int_t  [3*mNTrings]; mTringNorms = 0; mTringCols = 0;
  if(bSmoothShade) {
    mNorms = new Float_t[3*mNVerts];
    if(bColP) mCols = new UChar_t[4*mNVerts];
  } else {
    mTringNorms = new Float_t[3*mNTrings];
    if(bColP) mTringCols = new UChar_t[4*mNTrings];
  }
  if(bTexP) mTexs = new Float_t[2*mNVerts];

  mNStripEls = 0;
  mStripEls    = 0;
  mStripTrings = 0;
  mNStrips   = 0;
  mStripBegs = 0;
  mStripLens = 0;
}

TringTvor::~TringTvor()
{
  delete [] mVerts;    delete [] mNorms; delete [] mCols; delete [] mTexs;
  delete [] mTrings;   delete [] mTringNorms; delete [] mTringCols;
  delete [] mStripEls; delete [] mStripBegs; delete [] mStripLens;
}

/**************************************************************************/

void TringTvor::GenerateTriangleNormals()
{
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

  mNStripEls = cnt;
  mNStrips   = strip_list.size();

  mStripEls    = new Int_t[mNStripEls];
  mStripTrings = new Int_t[mNStripEls];
  mStripBegs = new Int_t*[mNStrips];
  mStripLens = new Int_t[mNStrips];

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
