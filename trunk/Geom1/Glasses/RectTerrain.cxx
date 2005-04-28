// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// RectTerrain
//
// A rectangular mNx * mNy height field.
// mO(x|y) ~ origin, mD(x|y) ~ grid spacing.
// Can be set from a ZImage via void SetFromImage(ZImage* image).
//
// Renderer is stupid, so expect it to be slow for large grids (512x512).

#include "RectTerrain.h"
#include <Glasses/ZImage.h>
#include "RectTerrain.c7"

#include <IL/il.h>

#include <stdio.h>

ClassImp(RectTerrain)

Float_t RectTerrain::sMaxEpsilon = 0.0001;

/**************************************************************************/

void RectTerrain::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;

  mNx = mNy = 0;
  mDx = mDy = 1;

  mMinZ = 0; mMaxZ = 0;
  mMinCol.gray(0.2); mMaxCol.gray(1);
  mColSep = 1;
  mRibbon = 0;

  mRnrMode    = RM_Histo;
  mBorderCond = BC_Zero;
  mOriginMode = OM_Edge;
  mBValue = 0;
  bBorder = false;

  mSmoothFac = 0.2;

  bStudySize   = false;

  pRTTvor = 0;
  bUseTringStrips = true;
}

RectTerrain::~RectTerrain()
{
  delete pRTTvor;
}

/**************************************************************************/

void RectTerrain::ApplyBorderCondition()
{
  const Int_t MX = mNx + 1;
  const Int_t MY = mNy + 1;
  const Int_t mx = mNx - 1;
  const Int_t my = mNy - 1;

  switch (mBorderCond) {

  case BC_Zero:
  case BC_Const: {
    const Float_t C = (mBorderCond == BC_Zero) ? 0 : mBValue;
    mP(0,0) = mP(MX,MY) = C;
    for(Int_t i=0; i<=MX; ++i) { mP(i, 0) = mP(i, MY) = C; }
    for(Int_t i=1; i< MY; ++i) { mP(0, i) = mP(MX, i) = C; }
    break;
  }
  case BC_External: {
    break;
  }
  case BC_Equal: {
    mP(0 ,0) = mP(1,1);   mP( 0,MY) = mP(1,mNy);
    mP(MX,0) = mP(mNx,1); mP(MX,MY) = mP(mNx,mNy);
    for(Int_t i=1; i<MX; ++i) { mP(i, 0) = mP(i,1); mP(i, MY) = mP(i,mNy); }
    for(Int_t i=1; i<MY; ++i) { mP(0, i) = mP(1,i); mP(MX, i) = mP(mNx,i); }
    break;
  }
  case BC_Sym: {
    mP(0 ,0) = mP(2,2);  mP( 0,MY) = mP(2,my);
    mP(MX,0) = mP(mx,2); mP(MX,MY) = mP(mx,my);
    for(Int_t i=1; i<MX; ++i) { mP(i, 0) = mP(i,2); mP(i, MY) = mP(i,my); }
    for(Int_t i=1; i<MY; ++i) { mP(0, i) = mP(2,i); mP(MX, i) = mP(mx,i); }
    break;
  }
  case BC_ASym: {
    mP(0 ,0) = 2*mP(1,1)   - mP(2,2);  mP( 0,MY) = 2*mP(1,  mNy) - mP(2,my);
    mP(MX,0) = 2*mP(mNx,1) - mP(mx,2); mP(MX,MY) = 2*mP(mNx,mNy) - mP(mx,my);
    for(Int_t i=1; i<MX; ++i) {
      mP(i, 0) = 2*mP(i,1) - mP(i,2); mP(i, MY) = 2*mP(i,mNy) - mP(i,my);
    }
    for(Int_t i=1; i<MY; ++i) {
      mP(0, i) = 2*mP(1,i) - mP(2,i); mP(MX, i) = 2*mP(mNx,i) - mP(mx,i);
    }
    break;
  }
  case BC_Wrap: {
    mP(0, 0) = mP(mNx,mNy); mP(0, MY) = mP(1,mNy);
    mP(MX,0) = mP(1,  mNy); mP(MX,MY) = mP(1,1);
    for(Int_t i=1; i<MX; ++i) { mP(i, 0) = mP(i,mNy); mP(i, MY) = mP(i,1); }
    for(Int_t i=1; i<MY; ++i) { mP(0, i) = mP(mNx,i); mP(MX, i) = mP(1,i); }
    break;
  }

  } // end switch mBCond

  mStampReqTring = Stamp(FID());
}

/**************************************************************************/

void RectTerrain::SetFromImage(ZImage* image)
{
  ZImage::sILMutex.Lock();
  image->bind();
  int w = image->w(), h = image->h();
  ILushort* data = new ILushort[w*h];
  ilCopyPixels(0,0,0, w,h,1, IL_LUMINANCE,IL_UNSIGNED_SHORT, data);
  ZImage::sILMutex.Unlock();

  mNx = w; mNy = h;
  mP.Clear();
  mP.ResizeTo(mNx + 2, mNy + 2);
  mMinZ = 1e9; mMaxZ = -1e9;

  for(Int_t y=1; y<=mNy; ++y) {
    ILushort* bar = &(data[(mNy-y)*w]);
    for(Int_t x=1; x<=mNx; ++x) {
      const float z = (float)(*(bar++));
      if(z > mMaxZ) mMaxZ = z;
      if(z < mMinZ) mMinZ = z;
      mP(x,y) = z;
    }
  }
  delete [] data;

  float zoffset = 0; // set z = 0 at 0
  float zfac = 1;    // scale not

  float delta = (1 + sMaxEpsilon)*(mMaxZ - mMinZ);
  for(Int_t x=1; x<=mNx; ++x) {
    for(Int_t y=1; y<=mNy; ++y) {
      mP(x,y) = ( (mP(x,y) - mMinZ)/delta - zoffset ) * zfac;
    }
  }
  
  if(zfac > 0) {
    mMinZ = -zoffset * zfac; mMaxZ = (1 - zoffset) * zfac;
  } else {
    mMaxZ = -zoffset * zfac; mMinZ = (1 - zoffset) * zfac;
  }

  mStampReqTring = Stamp(FID());
}

void RectTerrain::SetFromHisto(TH2* histo)
{
  mNx = histo->GetNbinsX(); mNy = histo->GetNbinsY();
  const Int_t MX = mNx + 1, MY = mNy + 1;
  mP.Clear();
  mP.ResizeTo(mNx+2, mNy+2);
  mMinZ = 1e9; mMaxZ = -1e9;

  for(Int_t x=0; x<=MX; ++x) {
    for(Int_t y=0; y<=MY; ++y) {
      mP(x,y) = histo->GetBinContent(x,y);
    }
  }
  mMinZ = histo->GetMinimum();
  mMaxZ = histo->GetMaximum();

  float zoffset = 0; // set z = 0 at 0
  float zfac = 1;    // scale not

  float delta = 1.001*(mMaxZ - mMinZ);
  for(Int_t x=0; x<=MX; ++x) {
    for(Int_t y=0; y<=MY; ++y) {
      mP(x,y) = ( (mP(x,y) - mMinZ)/delta - zoffset ) * zfac;
    }
  }
  
  if(zfac > 0) {
    mMinZ = -zoffset * zfac; mMaxZ = (1 - zoffset) * zfac;
  } else {
    mMaxZ = -zoffset * zfac; mMinZ = (1 - zoffset) * zfac;
  }

  mStampReqTring = Stamp(FID());
}

/**************************************************************************/

void RectTerrain::Smooth(Float_t fac)
{
  TMatrixF M(mP);
  Double_t fa = (fac == 0) ? mSmoothFac : fac;
  Double_t fb = fa/TMath::Sqrt(2.0);
  Double_t normfac = 1/(1 + 4*fa + 4*fb);
  for(Int_t i=1; i<=mNx; ++i) {
    for(Int_t j=1; j<=mNy; ++j) {
      mP(i,j) = (M(i,j) + fa*(M(i-1,j) + M(i+1,j) + M(i,j-1) + M(i,j+1)) +
                      fb*(M(i-1,j-1) + M(i+1,j+1) + M(i+1,j-1) + M(i-1,j+1))
		 ) * normfac;
    }
  }
  mStampReqTring = Stamp(FID());
}

void RectTerrain::RecalcMinMax()
{
  mMinZ = 1e9; mMaxZ = -1e9;
  for(Int_t i=1; i<=mNx; ++i) {
    for(Int_t j=1; j<=mNy; ++j) {
      if(mP(i,j) > mMaxZ) mMaxZ = mP(i,j);
      if(mP(i,j) < mMinZ) mMinZ = mP(i,j);
    }
  }
  mMaxZ *= (1 + sMaxEpsilon);
  mStampReqTring = Stamp(FID());
}

/**************************************************************************/

void RectTerrain::ReTring()
{
  mStampReqTring = Stamp(FID());
}

#include <TRandom.h>
#include <TVirtualMutex.h>

void RectTerrain::Boobofy()
{
  // Must set Ribbon to booby.pov for best effect.

  TH2F* h=new TH2F("Booboo","exampul",128,-5,5,64,-2.5,2.5);
  TRandom r;
  for(int i=0;i<1000000;++i) {
    h->Fill(r.Gaus() - 2, r.Gaus());
    h->Fill(r.Gaus() + 2, r.Gaus());
  }
  for(int i=0;i<6000;++i) {
    h->Fill(0.1*r.Gaus() - 2, 0.1*r.Gaus());
    h->Fill(0.1*r.Gaus() + 2, 0.1*r.Gaus());
  }
  {
    R__LOCKGUARD(gCINTMutex);
    h->Draw("LEGO2");
  }
  SetFromHisto(h);
  for(int i=0; i<5; ++i) Smooth(1);
  RecalcMinMax();
}

/**************************************************************************/

void RectTerrain::Tringoo()
{
  static const string _eh("RectTerrain::Tringoo ");


  Int_t minX = 1, maxX = mNx, minY = 1, maxY = mNy;
  if(bBorder) {
    --minX; ++maxX; --minY; ++maxY;
  }
  Int_t nx = maxX - minX + 1;
  Int_t ny = maxY - minY + 1;
  
  delete pRTTvor;
  pRTTvor = new RectTringTvor(nx*ny, (nx-1)*(ny-1)*2);
  RectTringTvor& RTT = *pRTTvor;

  TVector3 normvec;
  Int_t          idx = 0;
  Int_t    tring_idx = 0;

  for(Int_t j=minY; j<=maxY; ++j) {
    for(Int_t i=minX; i<=maxX; ++i) {

      Float_t *v = RTT.Vertex(idx);
      v[0] = (i-1)*mDx; v[1] = (j-1)*mDy; v[2] = mP[i][j];

      Int_t il=i,ih=i,jh=j,jl=j;
      if(i>0) il--; if(i<=mNx) ih++;
      if(j>0) jl--; if(j<=mNy) jh++;
      Float_t dvx[] = { (ih-il)*mDx, 0, mP(ih,j) - mP(il,j) };
      Float_t dvy[] = { 0, (jh-jl)*mDy, mP(i,jh) - mP(i,jl) };
      normvec.SetXYZ(-dvy[1]*dvx[2], -dvx[0]*dvy[2], dvx[0]*dvy[1]);
      normvec.SetMag(1);
      Float_t *n = RTT.Normal(idx);
      n[0] = normvec.x(); n[1] = normvec.y(); n[2] = normvec.z();

      {
	if(mColSep) {
	  Float_t c = (mP[i][j] - mMinZ) * mColSep / (mMaxZ - mMinZ);
	  c -= (int)c;
	  ZColor col( mRibbon ?
		      mRibbon->MarkToCol(c) :
		      (1 - c)*mMinCol + c*mMaxCol );
	  col.rgb_to_ubyte(RTT.Color(idx));
	}
      }

      if(i < maxX && j < maxY) {
	Float_t d1 = TMath::Abs(mP[i][j] - mP[i+1][j+1] );
	Float_t d2 = TMath::Abs(mP[i][j+1] - mP[i+1][j] );
	Int_t* T = RTT.Triangle(tring_idx);
	//if(j < 2) {
	//printf("j=%3d i=%3d idx=%4d tidx=%4d T=%p\n",
	// j, i, idx, tring_idx, T);
	//}
	if(d1 > d2) {
	  T[0] = idx;   T[1] = idx+1;    T[2] = idx+nx;   T += 3;
	  T[0] = idx+1; T[1] = idx+nx+1; T[2] = idx+nx;
	} else {
	  T[0] = idx;   T[1] = idx+1;    T[2] = idx+nx+1; T += 3;
	  T[0] = idx;   T[1] = idx+nx+1; T[2] = idx+nx;
	}
	tring_idx += 2;
      }

      ++idx;
    }
  }

  RTT.GenerateTriangleNormals();
  RTT.GenerateTriangleStrips();

  // mStampReqTring = Stamp(FID());
}

/**************************************************************************/
// RectTringTvor
/**************************************************************************/

void RectTringTvor::GenerateTriangleNormals()
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

void RectTringTvor::GenerateTriangleStrips()
{
  static const string _eh("RectTringTvor::GenerateTriangleStrips ");

  hash_map<xx_tring, Int_t> tring_map;

  ACTCData *tc = actcNew();
  if(tc == 0) throw(_eh + "failed to allocate TC structure.");
  // actcParami(tc, ACTC_OUT_MIN_FAN_VERTS, is maxint); // 
  // actcParami(tc, ACTC_OUT_MAX_PRIM_VERTS, 128);
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
