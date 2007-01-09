// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringuCam_H
#define Var1_TringuCam_H

#include <Glasses/Camera.h>
#include <Glasses/ScreenText.h>
#include <Glasses/Tringula.h>
#include <Stones/TimeMakerClient.h>
#include <Gled/GTime.h>

class TringuCam : public Camera, public TimeMakerClient
{
  MAC_RNR_FRIENDS(TringuCam);

public:

  struct ValueInfo;

  struct KeyInfo
  {
    ValueInfo* fValueInfo;
    Bool_t     fIsInc;

    Bool_t  fIsDown;
    Float_t fDesiredValue; // Always positive.
    Float_t fDecayTimeout;

    KeyInfo(ValueInfo* val_info, Bool_t is_inc) : 
      fValueInfo(val_info), fIsInc(is_inc),
      fIsDown(false), fDesiredValue(0), fDecayTimeout(0) {}
  };

  struct KeyValueChangeParams
  {
    Float_t fValueAccel;
    Float_t fValueAccelDeltaFactor;
    Float_t fValueDecay;
    Float_t fValueDecayDeltaFactor;
    Float_t fValueDecayTimeout;

    Float_t fDesireIncStep;
    Float_t fDesireIncDeltaFactor; // This should really link to number of keydowns / pressure if had key-velocity info
    Float_t fDesireDecay;
    Float_t fDesireDecayDeltaFactor;
    Float_t fDesireDecayTimeout;

    void SetValueParams(Float_t acc,   Float_t acc_df,
                        Float_t decay, Float_t decay_df,
                        Float_t decay_to)
    {
      fValueAccel = acc;   fValueAccelDeltaFactor = acc_df;
      fValueDecay = decay; fValueDecayDeltaFactor = decay_df;
      fValueDecayTimeout = decay_to;
    }
    void SetDesireParams(Float_t inc_step, Float_t inc_df,
                         Float_t decay,    Float_t decay_df,
                         Float_t decay_to)
    {
      fDesireIncStep = inc_step; fDesireIncDeltaFactor   = inc_df;
      fDesireDecay   = decay;    fDesireDecayDeltaFactor = decay_df;
      fDesireDecayTimeout = decay_to;
    }
  };

  struct ValueInfo
  {
    KeyValueChangeParams* fChangeParams;

    Float_t fValue;
    Float_t fDecayTimeout;
    KeyInfo fIncKey;
    KeyInfo fDecKey;

    ValueInfo(KeyValueChangeParams* chg_prm=0) :
      fChangeParams(chg_prm),
      fValue(0), fDecayTimeout(0),
      fIncKey(this, true), fDecKey(this, false) {}

    void IncValue(Float_t& value, Float_t desire, Float_t step, Float_t delta_fac);
    void DecValue(Float_t& value, Float_t desire, Float_t step, Float_t delta_fac);

    void DecayValue(Float_t& value, Float_t decay, Float_t delta_fac=0);

    void DecayTimeoutOrValue(Float_t& timeout, Float_t& value, Float_t dt,
                             Float_t decay, Float_t delta_fac);

    void TimeTick(Float_t dt);
  };

  enum KeyPairs_e { KP_StrafeLR };

private:
  void _init();

protected:
  KeyValueChangeParams mChgParCameraMove;
  KeyValueChangeParams mChgParCameraRotate;

  ValueInfo mFwdBck;
  ValueInfo mLftRgt;
  ValueInfo mUpDown;

  ValueInfo mSpinUp; // Spin about parent's up axis

  map<Int_t, KeyInfo*> mKeyStateMap;

  ZLink<ScreenText>  mTxtLftRgt; // X{GS} L{}

public:
  TringuCam(const Text_t* n="TringuCam", const Text_t* t=0) :
    Camera(n,t) { _init(); }

  KeyInfo* FindKeyInfo(Int_t key);
  Int_t KeyDown(Int_t key);
  Int_t KeyUp(Int_t key);

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

#include "TringuCam.h7"
  ClassDef(TringuCam, 1)
}; // endclass TringuCam


#endif
