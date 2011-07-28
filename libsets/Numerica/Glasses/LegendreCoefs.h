// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_LegendreCoefs_H
#define GledCore_LegendreCoefs_H

#include <Glasses/ZGlass.h>

class HPointD;

class LegendreCoefs : public ZGlass
{
  MAC_RNR_FRIENDS(LegendreCoefs);

public:
  struct Evaluator
  {
    const LegendreCoefs *fCoefs;
    Double_t             fScale;
    Int_t                fLMax;

    Evaluator(const LegendreCoefs* lc, Double_t s=1, Int_t lm=-1) :
      fCoefs(lc), fScale(s), fLMax(lm) {}

    Double_t Eval(Double_t cos_theta, Double_t phi) const;
    Double_t Eval(const HPointD& vec) const;
  };

  struct MultiEval
  {
    vector<Double_t> fMVec; // |cos-theta| on input, legend sum on output
    vector<Double_t> fPhis;
    vector<Int_t>    fIdcs;
    vector<void*>    fUserData;
    Int_t            fN;

    MultiEval() {}

    void Init(Int_t n);

    void AddPoint     (Double_t cos_theta, Double_t phi, void* ud);
    void AddPoint     (Double_t x, Double_t y, Double_t z, void* ud);
    void AddPointUnitR(Double_t x, Double_t y, Double_t z, void* ud);

    void Sort();
  };

private:
  void _init();

protected:
  Int_t            mLMax; // X{G} 7 ValOut()
  vector<Double_t> mC;

  Double_t& coef(Int_t l, Int_t m) { return mC[l*l + l + m]; }

  Double_t sum_m(Int_t l, Int_t m, Double_t cos_mphi, Double_t sin_mphi) const
  {
    if (m == 0) {
      return mC[l*l + l];
    } else {
      Int_t idx = l*l + l - m;
      return mC[idx + 2*m] * cos_mphi + mC[idx] * sin_mphi;
    }
  }

public:
  LegendreCoefs(const Text_t* n="LegendreCoefs", const Text_t* t=0);
  virtual ~LegendreCoefs();

  void InitRandom(Int_t l_max, Double_t abs_scale=0.1, Double_t pow_scale=2.7); // X{E} 7 MCWButt()
  void InitToValue(Int_t l_max, Double_t value=0); // X{E} 7 MCWButt()
  void SetCoef(Int_t l, Int_t m, Double_t v);      // X{E} 7 MCWButt()

  void ReadEgmFile(const TString& egm, Int_t l_max); // X{E} 7 MCWButt()

  Double_t Coef(Int_t l, Int_t m) const { return mC[l*l + l + m]; }

  Double_t Eval(Double_t cos_theta, Double_t phi, Int_t l_max=-1) const;
  Double_t Eval(const HPointD& vec, Int_t l_max=-1) const;

  void EvalMulti(MultiEval& me, Int_t l_max) const;

  void MakeRandomSamplingHisto(Int_t max_l=-1, Int_t n_samples=10000,
			       const TString& canvas_name  = "RndSampling",
			       const TString& canvas_title = "Distribution of LegendreCoef values over random points on sphere"); //! X{ED} 7 MCWButt()

#include "LegendreCoefs.h7"
  ClassDef(LegendreCoefs, 1);
}; // endclass LegendreCoefs

#endif
