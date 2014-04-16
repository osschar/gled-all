// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ParaSurf.h"
#include "TriMesh.h"
#include "ParaSurf.c7"
#include <Stones/HTrans.h>
#include <Stones/ZTrans.h>

#include <Opcode/Opcode.h>

#include <TMath.h>
#include <TRandom.h>


//==============================================================================
// ParaSurf
//==============================================================================

//__________________________________________________________________________
//
// Virtual base-class for describing the coordinate system of a 2D
// parametric surface. The surface is described with two parameters
// (f, g) and the third one accounts for distance in the normal
// direction (h, height).
//
// Methods in this class allow for mapping between parametric (f,g,h)
// space and world (x,y,z) space.

ClassImp(ParaSurf);

const Float_t ParaSurf::sEpsilonFac  = 1e-5;
const Float_t ParaSurf::sPi          = 3.14159265358979323846f;
const Float_t ParaSurf::sTwoPi       = 6.28318530717958647692f;
const Float_t ParaSurf::sPiHalf      = 1.57079632679489661923f;

/**************************************************************************/

void ParaSurf::_init()
{
  mMinF = mMaxF = mMinG = mMaxG = mMinH = mMaxH = mMeanH = mSigmaH = 0;
  mGravAtSurface = 10.0;
  mEpsilon = sEpsilonFac;
}

/**************************************************************************/

void ParaSurf::FindMinMaxFGH(TriMesh* mesh)
{
  static const Exc_t _eh("ParaSurf::FindMinMaxFGH ");

  if (mesh == 0) throw(_eh + "mesh null.");

  TringTvor & TT = * mesh->GetTTvor();
  if (TT.mNVerts < 1)
  {
    mMinF = mMaxF = mMinG = mMaxG = mMinH = mMaxH = mMeanH = mSigmaH = 0;
    return;
  }

  mMinF = mMinG = mMinH =  FLT_MAX;
  mMaxF = mMaxG = mMaxH = -FLT_MAX;
  Double_t sumh = 0, sumh2 = 0;

  Float_t * V  = TT.Verts();
  Int_t     N  = TT.mNVerts;
  Float_t   F[3];
  for (Int_t i=0; i<N; ++i, V+=3)
  {
    pos2fgh(V, F);
    if (F[0] < mMinF) mMinF = F[0]; else if (F[0] > mMaxF) mMaxF = F[0];
    if (F[1] < mMinG) mMinG = F[1]; else if (F[1] > mMaxG) mMaxG = F[1];
    if (F[2] < mMinH) mMinH = F[2]; else if (F[2] > mMaxH) mMaxH = F[2];
    sumh += F[2]; sumh2 += F[2]*F[2];
  }
  mMeanH   = sumh/N;
  mSigmaH  = TMath::Sqrt(TMath::Abs(sumh2/N - sumh*sumh/N/N));
  mEpsilon = sEpsilonFac*(mMaxH - mMinH);

  Stamp(FID());
}

void ParaSurf::FindMinMaxH(TriMesh* mesh)
{
  static const Exc_t _eh("ParaSurf::FindMinMaxH ");

  if (mesh == 0) throw(_eh + "mesh null.");

  TringTvor & TT = * mesh->GetTTvor();
  if (TT.mNVerts < 1)
  {
    mMinH = mMaxH = mMeanH = mSigmaH = 0;
    return;
  }

  mMinH =  FLT_MAX;
  mMaxH = -FLT_MAX;
  Double_t sumh = 0, sumh2 = 0;

  Float_t * V  = TT.Verts();
  Int_t     N  = TT.mNVerts;
  Float_t   F[3];
  for (Int_t i=0; i<N; ++i, V+=3)
  {
    pos2fgh(V, F);
    if (F[2] < mMinH) mMinH = F[2]; else if (F[2] > mMaxH) mMaxH = F[2];
    sumh += F[2]; sumh2 += F[2]*F[2];
  }
  mMeanH   = sumh/N;
  mSigmaH  = TMath::Sqrt(TMath::Abs(sumh2/N - sumh*sumh/N/N));
  mEpsilon = sEpsilonFac*(mMaxH - mMinH);

  Stamp(FID());
}

void ParaSurf::RandomizeH(TriMesh* mesh,
                          Float_t  fg_chr_len, Float_t h_chr_len,
                          Float_t  alpha,      Bool_t  accum_dh,
                          Float_t  n_pass)
{
  // Randomize vertex heights by fractional brownian rules.
  // Does random selection of vertex-pairs, number of edges visited in
  // each pass is estimated as 3 * Nvert.
  // fg_chr_len - characteristic length in f,g direction
  // h_chr_len  - characteristic length in h direction

  static const Exc_t _eh("ParaSurf::RandomizeH ");

  if (mesh == 0) throw(_eh + "mesh null.");

  using namespace Opcode;

  TringTvor& TT = * mesh->GetTTvor();
  Int_t  NV = TT.mNVerts;
  Int_t  NT = TT.mNTrings;
  Point* X  = (Point*) TT.Verts();
  Point* U  = new Point [NV];

  // Calculate up-dirs for each vertex.
  for (Int_t i=0; i<NV; ++i)
    pos2hdir(X[i], U[i]);

  TRandom rnd(0);
  const Int_t   n_max      = Int_t( 3*NV*n_pass );
  const Float_t fg_fac     = 1/fg_chr_len/fg_chr_len;
  const Float_t alpha_half = 0.5*alpha;

  for (Int_t i = 0; i < n_max; ++i)
  {
    Int_t  *vs  = TT.Triangle( rnd.Integer(NT) );
    Int_t   v1  = vs[ rnd.Integer(3) ];
    Int_t   v2  = vs[ v1 < 2 ? v1 + 1 : 0 ];

    Point   n   = U[v1] + U[v2];  n.Normalize();
    Point   dx  = X[v2] - X[v1];
    Float_t odh = dx | n;
    dx.TMsc(n, odh);
    // get random delta h from gaussian ...
    Float_t dfgsq = fg_fac * dx.SquareMagnitude();
    Float_t sgm   = h_chr_len * powf(dfgsq, alpha_half);
    Float_t dh    = rnd.Gaus(0, sgm);
    Float_t hh    = 0.5 * (accum_dh ? dh : dh + odh);
    X[v1].TMac(U[v1], hh);
    X[v2].TMsc(U[v2], hh);
  }

  delete [] U;
}

/**************************************************************************/

Bool_t ParaSurf::IsValidFGH(const Float_t f[3], Bool_t check_h)
{
  if (f[0] < mMinF || f[0] > mMaxF || f[1] < mMinG || f[1] > mMaxG)
    return false;
  if (check_h && (f[2] < mMinH || f[2] > mMaxH))
    return false;
  return true;
}

Bool_t ParaSurf::IsValidPos(const Float_t x[3], Bool_t check_h)
{
  Float_t f[3];
  pos2fgh(x, f);
  return IsValidFGH(f);
}

/**************************************************************************/

Float_t ParaSurf::CharacteristicLength()
{
  return sqrtf(Surface());
}

/**************************************************************************/

void ParaSurf::origin_fgh(Float_t* f)
{
  f[0] = f[1] = f[2] = 0.0f;
}

void ParaSurf::origin_pos(Float_t* x)
{
  Float_t fgh[3];
  origin_fgh(fgh);
  fgh2pos(fgh, x);
}

void ParaSurf::origin_trans(HTransF& t)
{
  Float_t fgh[3];
  origin_fgh(fgh);
  fgh2trans(fgh, t);
}

void ParaSurf::origin_trans(ZTrans& t)
{
  HTransF ht;
  origin_trans(ht);
  t.SetFromArray(ht.Array());
}

/******************************************************************************/

void ParaSurf::fgh2trans(const Float_t* f, HTransF& t)
{
  fgh2fdir(f, t.ArrX());
  fgh2gdir(f, t.ArrY());
  fgh2hdir(f, t.ArrZ());
  fgh2pos (f, t.ArrT());
}

void ParaSurf::fgh2trans(const Float_t* f, ZTrans& t)
{
  HTransF ht;
  fgh2trans(f, ht);
  t.SetFromArray(ht.Array());
}

/******************************************************************************/

void ParaSurf::pos2fghdir(const Float_t* x, Float_t* fdir, Float_t* gdir, Float_t* hdir)
{
  Float_t fgh[3];
  pos2fgh(x, fgh);
  fgh2fdir(fgh, fdir);
  fgh2gdir(fgh, gdir);
  fgh2hdir(fgh, hdir);
}

void ParaSurf::sub_fgh(Float_t* a, Float_t* b, Float_t* delta)
{
  // Subtract fgh values, taking care of U(1) variables (like angles).
  // This base-class version does simple delta = a - b, which is ok for
  // PSRectangle and PSTriangle.

  delta[0] = a[0] - b[0];
  delta[1] = a[1] - b[1];
  delta[2] = a[2] - b[2];
}

void ParaSurf::regularize_fg(Float_t* f)
{
  // Put fg values into regular intervals.
  // Here we just clamp them to min, max values which is ok for
  // PSRectangle and PSTriangle.

  if (f[0] < mMinF) f[0] = mMinF; else if (f[0] > mMaxF) f[0] = mMaxF;
  if (f[1] < mMinG) f[1] = mMinG; else if (f[1] > mMaxG) f[1] = mMaxG;
}

void ParaSurf::random_pos(TRandom& rnd, Float_t* x)
{
  Float_t fgh[3];
  random_fgh(rnd, fgh);
  fgh2pos(fgh, x);
}

void ParaSurf::random_trans(TRandom& rnd, HTransF& t)
{
  Float_t fgh[3];
  random_fgh(rnd, fgh);
  fgh2trans(fgh, t);
}
