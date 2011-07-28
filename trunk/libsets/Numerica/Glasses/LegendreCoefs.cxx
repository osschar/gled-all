// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "LegendreCoefs.h"
#include "LegendreCoefs.c7"

#include "Stones/HTrans.h"
#include "Gled/XTReqCanvas.h"

#include "TH1.h"
#include "TCanvas.h"

#include "TMath.h"
#include "TRandom3.h"

#include <gsl/gsl_sf_legendre.h>

// LegendreCoefs

//______________________________________________________________________________
//
//

ClassImp(LegendreCoefs);

//==============================================================================

void LegendreCoefs::_init()
{}

LegendreCoefs::LegendreCoefs(const Text_t* n, const Text_t* t) :
  ZGlass(n, t),
  mLMax(0)
{
  _init();
}

LegendreCoefs::~LegendreCoefs()
{}

//==============================================================================

void LegendreCoefs::InitRandom(Int_t l_max, Double_t abs_scale, Double_t pow_scale)
{
  static const Exc_t _eh("LegendreCoefs::InitRandom ");

  if (l_max < 0)
    throw _eh + "l_max must be non-negative.";

  mLMax = l_max;
  mC.resize((l_max + 1) * (l_max + 1));

  using namespace TMath;

  TRandom3 rnd(0);

  for (Int_t l = 0; l <= mLMax; ++l)
  {
    Double_t fl   = 0.25*(2*l + 1) / Pi();
    Double_t fpow = abs_scale * Power(1.0/(2*l + 1), pow_scale);

    Double_t& l0 = coef(l, 0);

    l0  = Sqrt(fl);
    l0 *= fpow * (2*rnd.Rndm() - 1);

    for (Int_t m = 1; m <= l; ++m)
    {
      Double_t& lpos = coef(l,  m);
      Double_t& lneg = coef(l, -m);

      lpos  = Sqrt(fl);
      lneg  = (m % 2) ? -lpos : lpos;
      lpos *= fpow * (2*rnd.Rndm() - 1);
      lneg *= fpow * (2*rnd.Rndm() - 1);
    }
  }

  Stamp(FID());
}

void LegendreCoefs::InitToValue(Int_t l_max, Double_t value)
{
  static const Exc_t _eh("LegendreCoefs::InitToValue ");

  if (l_max < 0)
    throw _eh + "l_max must be non-negative.";

  mLMax = l_max;

  vector<Double_t> new_c((l_max + 1) * (l_max + 1), value);
  mC.swap(new_c);

  Stamp(FID());
}

void LegendreCoefs::SetCoef(Int_t l, Int_t m, Double_t v)
{
  static const Exc_t _eh("LegendreCoefs::SetCoef ");

  if (l > mLMax || l < 0)
    throw _eh + "Incorrect argument 'l'.";
  if (m < -l || m > l)
    throw _eh + "Incorrect argument 'm'.";

  mC[l*l + l + m] = v;
}

//==============================================================================

void LegendreCoefs::ReadEgmFile(const TString& egm, Int_t l_max)
{
  static const Exc_t _eh("LegendreCoefs::ReadEgmFile ");

  FILE *fp = fopen(egm, "r");
  if (!fp)
    throw _eh + "can not open file '" + egm + "'.";

  {
    Int_t l_max_in_file;
    if (fscanf(fp, "%d", &l_max_in_file) != 1)
    {
      fclose(fp);
      throw _eh + "Failed reading l_max_in_file.";
    }
    if (l_max_in_file < l_max)
    {
      ISwarn(_eh + GForm("requested l_max=%d less then l_max_in_file=%d; using l_max_in_file.", l_max, l_max_in_file));
      l_max = l_max_in_file;
    }
  }

  mLMax = l_max;
  mC.resize((l_max + 1) * (l_max + 1));

  for (Int_t l = 0; l <= l_max; ++l)
  {
    Int_t idx = l*l + l;
    for (Int_t m = 0; m <= l; ++m)
    {
      Int_t    l_in,   m_in;
      Double_t cos_in, sin_in;

      if (fscanf(fp, "%d %d %lf %lf", &l_in, &m_in, &cos_in, &sin_in) != 4 ||
	  l_in != l || m_in != m)
      {
	fclose(fp);
	throw _eh + GForm("Failed at reading l=%d, m=%d.", l, m);
      }

      if (m == 0)
      {
	mC[idx] = cos_in;
      }
      else
      {
	mC[idx - m] = sin_in;
	mC[idx + m] = cos_in;
      }
    }
  }

  fclose(fp);

  Stamp(FID());
}

//==============================================================================

Double_t LegendreCoefs::Eval(Double_t cos_theta, Double_t phi, Int_t l_max) const
{
  static const Double_t sqrt4pi = TMath::Sqrt(4*TMath::Pi());

  if (l_max > mLMax || l_max < 0)
  {
    l_max = mLMax;
  }

  vector<Double_t> mvals(l_max + 1);
  Double_t sum = 0;

  for (Int_t m = 0; m <= l_max; ++m)
  {
    const Double_t cos_mphi = TMath::Cos(m*phi);
    const Double_t sin_mphi = TMath::Sin(m*phi);

    gsl_sf_legendre_sphPlm_array(l_max, m, cos_theta, &mvals[m]);

    for (Int_t l = m; l <= l_max; ++l)
    {
      sum += sum_m(l, m, cos_mphi, sin_mphi) * mvals[l];
    }
  }

  return sqrt4pi * sum;
}

Double_t LegendreCoefs::Eval(const HPointD& vec, Int_t l_max) const
{
  return Eval(vec.CosTheta(), vec.Phi(), l_max);
}

//------------------------------------------------------------------------------

void LegendreCoefs::EvalMulti(MultiEval& me, Int_t l_max) const
{
  static const Double_t sqrt4pi = TMath::Sqrt(4*TMath::Pi());

  if (l_max > mLMax || l_max < 0)
  {
    l_max = mLMax;
  }

  vector<Double_t> mvals(l_max + 1);

  Double_t cos_theta;
  Int_t n1, n2 = 0;

  while (n2 < me.fN)
  {
    n1 = n2;
    cos_theta = me.fMVec[me.fIdcs[n1]];
    me.fMVec[me.fIdcs[n1]] = 0;
    n2 = n1 + 1;
    while (n2 < me.fN && me.fMVec[me.fIdcs[n2]] == cos_theta)
    {
      me.fMVec[me.fIdcs[n2]] = 0;
      ++n2;
    }

    for (Int_t m = 0; m <= l_max; ++m)
    {
      gsl_sf_legendre_sphPlm_array(l_max, m, cos_theta, &mvals[m]);

      for (Int_t i = n1; i < n2; ++i)
      {
	const Int_t    ii       = me.fIdcs[i];
	const Double_t phi      = me.fPhis[ii];
	const Double_t cos_mphi = TMath::Cos(m * phi);
	const Double_t sin_mphi = TMath::Sin(m * phi);

	if (phi > 0)
	{
	  for (Int_t l = m; l <= l_max; ++l)
	  {
	    me.fMVec[ii] += sum_m(l, m, cos_mphi, sin_mphi) * mvals[l];
	  }
	}
	else
	{
	  for (Int_t l = m; l <= l_max; ++l)
	  {
	    const Double_t val = sum_m(l, m, cos_mphi, sin_mphi) * mvals[l];
	    if (l % 2 == 0)
	      me.fMVec[ii] += val;
	    else
	      me.fMVec[ii] -= val;
	  }
	}
      }
    }

    for (Int_t i = n1; i < n2; ++i)
    {
      const Int_t ii = me.fIdcs[i];
      me.fMVec[ii] *= sqrt4pi;
    }
  }
}

//==============================================================================

void LegendreCoefs::MakeRandomSamplingHisto(Int_t max_l, Int_t n_samples,
					    const TString& canvas_name,
					    const TString& canvas_title)
{
  TRandom3 rnd(0);

  TH1I *h = new TH1I("RndSampling", "Elevation", 256, 0, 0);
  h->SetBuffer(TMath::Max(10000, n_samples/10));

  for (Int_t i = 0; i < n_samples; ++i)
  {
    using namespace TMath;
    const Double_t cos_theta = rnd.Uniform(-1, 1);
    const Double_t phi       = rnd.Uniform(-Pi(), Pi());
    h->Fill(Eval(cos_theta, phi, max_l));
  }

  TCanvas *canvas = XTReqCanvas::Request(canvas_name, canvas_title);
  canvas->cd();
  h->Draw();
  XTReqPadUpdate::Update(canvas);
}


//==============================================================================
// LegendreCoefs::Evaluator
//==============================================================================

Double_t LegendreCoefs::Evaluator::Eval(Double_t cos_theta, Double_t phi) const
{
  return fScale * fCoefs->Eval(cos_theta, phi, fLMax);
}

Double_t LegendreCoefs::Evaluator::Eval(const HPointD& vec) const
{
  return Eval(vec.CosTheta(), vec.Phi());
}


//==============================================================================
// LegendreCoefs::MultiEval
//==============================================================================

void LegendreCoefs::MultiEval::Init(Int_t n)
{
  fMVec.resize(n);
  fPhis.resize(n);
  fIdcs.resize(n);
  fUserData.resize(n);
  fN = 0;
}

void LegendreCoefs::MultiEval::AddPoint(Double_t cos_theta, Double_t phi, void* ud)
{
  if (cos_theta >= 0)
  {
    fMVec[fN] = cos_theta;
    fPhis[fN] = phi + TMath::TwoPi();
  }
  else
  {
    fMVec[fN] = -cos_theta;
    fPhis[fN] = phi - TMath::Pi();
  }
  fUserData[fN] = ud;
  ++fN; 
}

void LegendreCoefs::MultiEval::AddPoint(Double_t x, Double_t y, Double_t z, void* ud)
{
  Double_t mag = TMath::Sqrt(x*x + y*y + z*z);
  AddPoint((mag == 0) ? 1 : z / mag, TMath::ATan2(y, x), ud);
}

void LegendreCoefs::MultiEval::AddPointUnitR(Double_t x, Double_t y, Double_t z, void* ud)
{
  AddPoint(z, TMath::ATan2(y, x), ud);
}

void LegendreCoefs::MultiEval::Sort()
{
  TMath::Sort(fN, &fMVec[0], &fIdcs[0]);
}
