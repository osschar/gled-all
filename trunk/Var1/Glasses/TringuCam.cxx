// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TringuCam
//
//

#include "TringuCam.h"
#include "TringuCam.c7"

ClassImp(TringuCam)

/**************************************************************************/

void TringuCam::_init()
{
  //                               acc/inc  | decay     | timeout
  mChgParCameraMove.SetValueParams (1, 0.5,   1.2, 0.4,   0.05);
  mChgParCameraMove.SetDesireParams(1, 0.3,   1,   0.1,   0.2);

  mFwdBck.fChangeParams = &mChgParCameraMove;
  mKeyStateMap['w'] = &mFwdBck.fIncKey;
  mKeyStateMap['s'] = &mFwdBck.fDecKey;

  mLftRgt.fChangeParams = &mChgParCameraMove;
  mKeyStateMap['a'] = &mLftRgt.fIncKey;
  mKeyStateMap['d'] = &mLftRgt.fDecKey;

  mUpDown.fChangeParams = &mChgParCameraMove;
  mKeyStateMap['r'] = &mUpDown.fIncKey;
  mKeyStateMap['f'] = &mUpDown.fDecKey;

  //                                 acc/inc    | decay      | timeout
  mChgParCameraRotate.SetValueParams (0.1, 0.5,   1.2, 0.4,    0.05);
  mChgParCameraRotate.SetDesireParams(0.1, 0.3,   0.1, 0.1,    0.2);

  mSpinUp.fChangeParams = &mChgParCameraRotate;
  mKeyStateMap['q'] = &mSpinUp.fIncKey;
  mKeyStateMap['e'] = &mSpinUp.fDecKey;
 
}

/**************************************************************************/

inline TringuCam::KeyInfo* TringuCam::FindKeyInfo(Int_t key)
{
  map<Int_t, KeyInfo*>::iterator i = mKeyStateMap.find(key);
  return (i != mKeyStateMap.end()) ? i->second : 0;
}

Int_t TringuCam::KeyDown(Int_t key)
{
  KeyInfo* ki = FindKeyInfo(key);
  if (ki == 0) return 0;
  if (ki->fIsDown == false)
  {
    ki->fIsDown = true;

    ValueInfo* vi = ki->fValueInfo;
    KeyValueChangeParams& C = * vi->fChangeParams;
    // KeyInfo*  oki = ki->fIsInc ? &vi->fDecKey : &vi->fIncKey;

    // If pressed during timeout: inc + delta_factor
    if (ki->fDecayTimeout > 0)
    {
      ki->fDesiredValue += C.fDesireIncDeltaFactor * ki->fDesiredValue;
      ki->fDesiredValue += C.fDesireIncStep;
    } else {
    // else round-up.
      ki->fDesiredValue = TMath::Ceil(ki->fDesiredValue + 0.001);
    }
  }
  return 1;
}

Int_t TringuCam::KeyUp(Int_t key)
{
  KeyInfo* ki = FindKeyInfo(key);
  if (ki == 0) return 0;
  if (ki->fIsDown == true)
  {
    ki->fIsDown = false;

    ValueInfo* vi = ki->fValueInfo;
    KeyInfo*  oki = ki->fIsInc ? &vi->fDecKey : &vi->fIncKey;
    KeyValueChangeParams& C = * vi->fChangeParams;

    ki->fDecayTimeout = C.fDesireDecayTimeout;
    if (oki->fIsDown == false)
    {
      vi->fDecayTimeout = C.fValueDecayTimeout;
    }
  }
  return 1;
}

/**************************************************************************/

void TringuCam::TimeTick(Double_t t, Double_t dt)
{
  mFwdBck.TimeTick(dt);
  if (mFwdBck.fValue) MoveLF(1, dt*mFwdBck.fValue);

  mLftRgt.TimeTick(dt);
  if (mLftRgt.fValue) MoveLF(2, dt*mLftRgt.fValue);

  mUpDown.TimeTick(dt);
  if (mUpDown.fValue) MoveLF(3, dt*mUpDown.fValue);

  mSpinUp.TimeTick(dt);
  if (mSpinUp.fValue) RotateLF(1, 2, dt*mSpinUp.fValue);

  

  if (*mTxtLftRgt != 0)
    mTxtLftRgt->SetText
      (GForm("Des+ = %5.3f | V = %5.3f  | Des- = %5.3f\n",
             mLftRgt.fIncKey.fDesiredValue, mLftRgt.fValue, mLftRgt.fDecKey.fDesiredValue));
}


/**************************************************************************/
/**************************************************************************/
// TringuCam::ValueInfo
/**************************************************************************/

inline void
TringuCam::ValueInfo::IncValue(Float_t& value, Float_t desire,
                               Float_t  step,  Float_t delta_fac)
{
  if (value < desire)
  {
    if (delta_fac) step += delta_fac*(desire - value);
    value += step;
    if (value > desire) value = desire;
  }
}

inline void
TringuCam::ValueInfo::DecValue(Float_t& value, Float_t desire,
                               Float_t  step,  Float_t delta_fac)
{
  desire = -desire;
  if (value > desire)
  {
    if (delta_fac) step += delta_fac*(value - desire);
    value -= step;
    if (value < desire) value = desire;
  }
}

inline void
TringuCam::ValueInfo::DecayValue(Float_t& value, Float_t decay,
                                 Float_t delta_fac)
{
  if (value > 0)
  {
    if (delta_fac)      decay += delta_fac*value;
    if (value > decay)  value -= decay;
    else                value  = 0;
  }
  else
  {
    if (delta_fac)      decay -= delta_fac*value;
    if (-value > decay) value += decay;
    else                value  = 0;
  }
}

inline void
TringuCam::ValueInfo::DecayTimeoutOrValue(Float_t& timeout, Float_t& value,
                                          Float_t  dt,      Float_t  decay,
                                          Float_t  delta_fac)
{
  if (timeout != 0)
    DecayValue(timeout, dt);
  else if (value != 0)
    DecayValue(value, decay, delta_fac);
}

// ------------------------------------------------------------------------

void TringuCam::ValueInfo::TimeTick(Float_t dt)
{
  KeyValueChangeParams& C = * fChangeParams;

  Float_t val_acc        = dt*C.fValueAccel;
  Float_t val_acc_dfac   = dt*C.fValueAccelDeltaFactor;
  Float_t val_decay      = dt*C.fValueDecay;
  Float_t val_decay_dfac = dt*C.fValueDecayDeltaFactor;

  Float_t des_decay      = dt*C.fDesireDecay;
  Float_t des_decay_dfac = dt*0.1; // !!! KONST

  if (fIncKey.fIsDown && fDecKey.fIsDown)
  {
    if (fValue != 0) DecayValue(fValue, val_decay+val_acc, val_decay_dfac+val_acc_dfac);
  }
  else if ( ! fIncKey.fIsDown && ! fDecKey.fIsDown)
  {
    DecayTimeoutOrValue(fDecayTimeout, fValue, dt, val_decay, val_decay_dfac);

    DecayTimeoutOrValue(fIncKey.fDecayTimeout, fIncKey.fDesiredValue,
                        dt, des_decay, des_decay_dfac);
    DecayTimeoutOrValue(fDecKey.fDecayTimeout, fDecKey.fDesiredValue,
                        dt, des_decay, des_decay_dfac);
  }
  else
  {
    if (fIncKey.fIsDown)
    {
      IncValue(fValue, fIncKey.fDesiredValue, val_acc, val_acc_dfac);
      
      DecayTimeoutOrValue(fDecKey.fDecayTimeout, fDecKey.fDesiredValue,
                          dt, des_decay, des_decay_dfac);
    }
    else
    {
      DecValue(fValue, fDecKey.fDesiredValue, val_acc, val_acc_dfac);

      DecayTimeoutOrValue(fIncKey.fDecayTimeout, fIncKey.fDesiredValue,
                          dt, des_decay, des_decay_dfac);
    }
  }
}
