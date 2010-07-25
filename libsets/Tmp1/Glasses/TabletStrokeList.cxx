// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletStrokeList.h"
#include "TabletStrokeList.c7"
#include "TabletStroke.h"

#include "Gled/XTReqCanvas.h"
#include <TH1F.h>
#include <TH2F.h>
#include "TCanvas.h"

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
  bMakeCubHistos  = false;

  mHCubTimevsTime = mHCubTimevsN = 0;
  mHN = mHCubTime20 = mHDeriv =  mHDeriv20 = 0;
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

void TabletStrokeList::calculate_qubic_coeffs(const vSTabletPoint_t& vec, Int_t i,
					      CCoefs& c)
{
  const STabletPoint& pm = vec[i-1];
  const STabletPoint& p0 = vec[i];
  const STabletPoint& p1 = vec[i+1];
  const STabletPoint& pM = vec[i+2];

  const Float_t T  = p1.t - p0.t;
  const Float_t T2 = T * T;
  const Float_t T3 = T * T2;

  c.D = p0;
  c.C = (p1 - pm) / (p1.t - pm.t);

  const STabletPoint Q = p1 - T*c.C - c.D;
  const STabletPoint R = (pM - p0) / (pM.t - p0.t) - c.C;

  c.B = (T3*R - 3.0f*Q) / (T2*(2*T2 - 3.0f));
  c.A = (Q - T2*c.B) / T3;
}

void TabletStrokeList::MakeKDStuff()
{
  if (mKDTree)
    ClearKDStuff();

  Stepper<TabletStroke> stepper(this);
  {
    Int_t n_points = 0;
    while (stepper.step())
    {
      n_points += stepper->get_num_points();
    }
    mPointRefs.reserve(n_points);
    mArrX.reserve(n_points);
    mArrY.reserve(n_points);
    if (mAlgorithm == PA_CubicInterpolation)
      mCCoefs.reserve(n_points);
  }
  stepper.reset(this);

  while (stepper.step())
  {
    Int_t min, max;
    stepper->get_draw_range(min, max);
    for (Int_t i = min; i <= max; ++i)
    {
      STabletPoint &p = stepper->mPoints[i];
      mPointRefs.push_back(&p);
      mArrX.push_back(p.x);
      mArrY.push_back(p.y);

      if (mAlgorithm == PA_CubicInterpolation)
      {
	mCCoefs.push_back(CCoefs());
	if (i < max)
	  calculate_qubic_coeffs(stepper->mPoints, i, mCCoefs.back());
      }
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
    mCCoefs.clear();
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
  if (mKDTree == 0 || (mAlgorithm == PA_CubicInterpolation && mCCoefs.empty()))
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
    } while (Abs(V) > 1e-8 && ++N < 100);

    mSearchRad = TR + R;
  }
  printf("TabletStrokeList::GTSIsoBegin iso=%f, search-radius=%f\n", iso_value, mSearchRad);

  if (bMakeCubHistos && mAlgorithm == PA_CubicInterpolation)
  {
    mHN             = new TH1I("N", "Number iterations", 21, -0.5, 20.5);
    mHCubTimevsTime = new TH2I("CubTimevsTime", "Tcub vs Tlin", 1000, -4, 5, 1000, -4, 5);
    mHCubTimevsN    = new TH2I("CubTimevsN", "Tcub vs N", 21, -0.5, 20.5, 1000, -4, 5);
    mHCubTime20     = new TH1I("CubTime20", "Tcub at N=20", 1000, -4, 5);
    mHDeriv         = new TH1I("Deriv", "Derivative value x 1e6", 1000, -1, 1);
    mHDeriv20       = new TH1I("Deriv20", "Derivative value x 1e3 at N=20", 1000, -1, 1);
  }
}

namespace
{
  Double_t sqr(Double_t x) { return x*x; }
}

void TabletStrokeList::distance_derivatives(Float_t t, const HPointF& P, const CCoefs& c,
					    Float_t& d1, Float_t& d2)
{
  const HPointF &A = c.A, &B = c.B, &C = c.C, &D = c.D;
  const HPointF At  = A  * t;
  const HPointF At2 = At * t;
  const HPointF Bt  = B  * t;

  const HPointF pmP = t * (At2 + Bt + C) + D - P;
  const HPointF pd1 = 3.0f * At2 + 2.0f * Bt + C;
  const HPointF pd2 = 6.0f * At  + 2.0f * B;

  d1 = pmP.Dot(pd1);
  d2 = pmP.Dot(pd2) + pd1.Dot(pd1);
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

    const Double_t Dsqr = sqr(x - p.x) + sqr(y - p.y) + sqr(z - p.z);
    const Double_t R    = mWidth * TMath::Power(p.p, mPressureAlpha);
    const Double_t v    = TMath::Power(Dsqr / sqr(R), mPotentialExp);

    if (v > vm[0])
    {
      vm[1] = vm[0]; id[1] = id[0];
      vm[0] = v;     id[0] = *pi;
    }
    else if (v > vm[1])
    {
      vm[1] = v; id[1] = *pi;
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

      // Stroke boundary?
      if (a.t > b.t) return vm[0];

      HPointF      P(x, y, z);
      STabletPoint atob = b - a;
      HPointF      atop = P - a;
      Float_t      dp = atob.Dot(atop);

      // Check if we are off one or the other edge:
      if (dp <= 0) return vm[0];
      Float_t atob_mag  = atob.Mag();
      dp /= atob_mag;
      if (dp >= atob_mag) return vm[1];

      Float_t      f = dp / atob_mag;
      STabletPoint p = (1.0f - f)*a + f*b;

      const Double_t Dsqr = sqr(x - p.x) + sqr(y - p.y) + sqr(z - p.z);
      const Double_t R    = mWidth * TMath::Power(p.p, mPressureAlpha);

      return TMath::Power(Dsqr / sqr(R), mPotentialExp);
    }
    case PA_CubicInterpolation:
    {
      if (id[1] == -1 || TMath::Abs(id[0] - id[1]) != 1) return vm[0];

      Int_t i0, i1;
      if (id[0] < id[1]) { i0 = id[0]; i1 = id[1]; } else { i0 = id[1]; i1 = id[0]; }
      STabletPoint &a = * mPointRefs[i0];
      STabletPoint &b = * mPointRefs[i1];

      // Stroke boundary?
      if (a.t > b.t) return vm[0];

      // Use "linear" time as starting approximation.
      HPointF      P(x, y, z);
      STabletPoint atob = b - a;
      HPointF      atop = P - a;
      Float_t      dp = atob.Dot(atop) / atob.Mag2();

      CCoefs &c = mCCoefs[i0];
      Float_t T = dp * atob.t;
      Float_t d1, d2;
      Int_t N = 0;
      do
      {
	distance_derivatives(T, P, c, d1, d2);
	T -= d1 / d2;
      } while (TMath::Abs(d1) > 1e-7f && ++N < 20);

      if (bMakeCubHistos)
      {
	mHN->Fill(N);
	mHCubTimevsTime->Fill(dp, T/atob.t);
	mHCubTimevsN->Fill(N, T/atob.t);
	mHDeriv->Fill(1e6f*d1);
	if (N >= 20)
	{
	  mHCubTime20->Fill(T/atob.t);
	  mHDeriv20->Fill(1e3f*d1);
	}
      }

      if (T < 0 || T > atob.t) return vm[0];

      const Float_t T2 = T * T;
      STabletPoint  p  = T2*T * c.A + T2 * c.B + T * c.C + c.D;

      const Double_t Dsqr = sqr(x - p.x) + sqr(y - p.y) + sqr(z - p.z);
      const Double_t R    = mWidth * TMath::Power(p.p, mPressureAlpha);

      return TMath::Power(Dsqr / sqr(R), mPotentialExp);
    }
  }

  // gcc was complaining ...
  return 0;
}

void TabletStrokeList::GTSIsoEnd()
{
  if (bMakeCubHistos && mAlgorithm == PA_CubicInterpolation)
  {
    TCanvas *canvas = XTReqCanvas::Request("StrokeList", "StrokeList Cubics", 1536, 768, 3, 2);
    canvas->cd(1); mHN->Draw();
    canvas->cd(2); mHCubTimevsTime->Draw();
    canvas->cd(3); mHCubTimevsN->Draw();
    canvas->cd(4); mHCubTime20->Draw();
    canvas->cd(5); mHDeriv->Draw();
    canvas->cd(6); mHDeriv20->Draw();
    XTReqPadUpdate::Update(canvas);
  }
}
