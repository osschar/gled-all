// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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

  bUseDispList = true;
  bStudySize   = false;
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

  mStampReqTring = Stamp(LibID(), ClassID());
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

  mStampReqTring = Stamp(LibID(), ClassID());
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

  mStampReqTring = Stamp(LibID(), ClassID());
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
  mStampReqTring = Stamp(LibID(), ClassID());
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
  mStampReqTring = Stamp(LibID(), ClassID());
}

/**************************************************************************/

void RectTerrain::ReTring()
{
  mStampReqTring = Stamp(LibID(), ClassID());
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
