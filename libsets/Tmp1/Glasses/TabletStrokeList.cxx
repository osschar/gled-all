// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletStrokeList.h"
#include "TabletStrokeList.c7"
#include "TabletStroke.h"
#include <Stones/STabletPoint.h>

// TabletStrokeList

//______________________________________________________________________________
//
// Potential is really (distance-from-point)^2
//
// mWidth         - radius where potential is one (for full-pressure)
// mPotentialExp  - exponent applied to this potential, usually negative
// mPressureAlpha - exponent of the pressure curve

ClassImp(TabletStrokeList);

//==============================================================================

void TabletStrokeList::_init()
{
  mAlgorithm      =  PA_TwoStrongestPoints;
  mWidth          =  0.01;
  mPotentialExp   = -0.5;
  mPressureAlpha  =  2;
  mMaxInterPoint  =  0.015;
}

TabletStrokeList::TabletStrokeList(const Text_t* n, const Text_t* t) :
  ZNode(n, t),
  mKDTree(0)
{
  _init();
}

TabletStrokeList::~TabletStrokeList()
{
  ClearKDStuff();
}

//==============================================================================

void TabletStrokeList::MakeKDStuff()
{
  if (mKDTree)
    ClearKDStuff();

  Stepper<TabletStroke> stepper(this);
  while (stepper.step())
  {
    Int_t min, max;
    stepper->get_draw_range(min, max);
    Int_t n_new = mPointRefs.size() + max - min + 1;
    mPointRefs.reserve(n_new);
    mArrX.reserve(n_new);
    mArrY.reserve(n_new);
    for (Int_t i = min; i <= max; ++i)
    {
      STabletPoint &p = stepper->mPoints[i];
      mPointRefs.push_back(&p);
      mArrX.push_back(p.x);
      mArrY.push_back(p.y);
    }
  }

  mKDTree = new TKDTreeIF(mArrX.size(), 2, 1);
  mKDTree->SetData(0, &mArrX[0]);
  mKDTree->SetData(1, &mArrY[0]);
  mKDTree->Build();  
}

void TabletStrokeList::ClearKDStuff()
{
  if (mKDTree)
  {
    delete mKDTree; mKDTree = 0;
    mPointRefs.clear();
    mArrX.clear();
    mArrY.clear();
  }
}

//==============================================================================

void TabletStrokeList::PrintClose(Float_t x, Float_t y, Float_t rad)
{
  if (mKDTree == 0)
    return;

  Float_t       point[2] = { x, y };
  vector<Int_t> res;

  mKDTree->FindInRange(point, rad, res);

  printf("TabletStrokeList::PrintClose npoints=%d, all stored=%d\n",
	 (Int_t) res.size(), (Int_t) mPointRefs.size());
}

//==============================================================================

void TabletStrokeList::GTSIsoBegin(Double_t iso_value)
{
  if (mKDTree == 0)
    MakeKDStuff();

  using namespace TMath;

  // Find distance where two consequitive points just make up the iso surface.
  // Standard maximum distance in tablet space is 0.01, default we take is 0.015.
  {
    const Double_t TR = mMaxInterPoint;
    const Double_t TE = 2.0*mPotentialExp;
    const Double_t C  = iso_value * Power(mWidth, TE);
    Double_t R = mWidth;
    Double_t V, D;
    Int_t N = 0;
    do
    {
      V = Power(R, TE) + Power(R + TR, TE) - C;
      D = TE * (Power(R, TE - 1) + Power(R + TR, TE - 1));
      // printf("Tangent method: R=%g, V=%g, D=%g\n", R, V, D);
      R -= V / D;
      ++N;
    } while (Abs(V) > 1e-8 && N < 100);

    mSearchRad = TR + R;
  }
  printf("TabletStrokeList::GTSIsoBegin iso=%f, search-radius=%f\n", iso_value, mSearchRad);
}

namespace
{
  Double_t sqr(Double_t x) { return x*x; }
}

Double_t TabletStrokeList::GTSIsoFunc(Double_t x, Double_t y, Double_t z)
{
  Float_t       in_point[2] = { x, y };
  vector<Int_t> result;

  mKDTree->FindInRange(in_point, mSearchRad, result);

  if (result.empty()) return 0;

  Int_t    id[2] = { -1, -1 };
  Double_t vm[2] = {  0,  0 };

  for (vector<Int_t>::iterator pi = result.begin(); pi != result.end(); ++pi)
  {
    STabletPoint &p = * mPointRefs[*pi];

    const Double_t Dsqr = sqr(x - p.x) + sqr(y - p.y) + sqr(z);
    const Double_t R    = mWidth * TMath::Power(p.p, mPressureAlpha);
    const Double_t v    = TMath::Power(Dsqr / sqr(R), mPotentialExp);

    if (v > vm[0])
    {
      vm[1] = vm[0]; id[1] = id[0];
      vm[0] = v;     id[0] = *pi;
    }
    else if (v > vm[1])
    {
      vm[1] = v;     id[1] = *pi;
    }
  }

  switch (mAlgorithm)
  {
    case PA_StrongestPoint:
    {
      return vm[0];
    }
    case PA_TwoStrongestPoints:
    {
      return vm[0] + vm[1];
    }
    case PA_LinearInterpolation:
    {
      if (id[1] == -1 || TMath::Abs(id[0] - id[1]) != 1) return vm[0];

      STabletPoint &a = * mPointRefs[id[0]];
      STabletPoint &b = * mPointRefs[id[1]];

      STabletPoint atob = b - a;
      HPointF      atop(x, y, z); atop -= a;
      Float_t      dp = atob.Dot(atop);

      // Check if we are off one or the other edge:
      if (dp <= 0) return vm[0];
      Float_t atob_mag  = atob.Mag();
      dp /= atob_mag;
      if (dp >= atob_mag) return vm[1];

      Float_t f = dp / atob_mag;
      STabletPoint p = (1.0f - f)*a + f*b;

      const Double_t Dsqr = sqr(x - p.x) + sqr(y - p.y) + sqr(z);
      const Double_t R    = mWidth * TMath::Power(p.p, mPressureAlpha);

      return TMath::Power(Dsqr / sqr(R), mPotentialExp);
    }
  }

  // gcc was complaining ...
  return 0;
}

void TabletStrokeList::GTSIsoEnd()
{}
