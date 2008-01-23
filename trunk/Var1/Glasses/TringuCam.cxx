// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TringuCam
//
//

#include "TringuCam.h"
#include "TriMeshField.h"
#include "TriMeshLightField.h"
#include <Glasses/ScreenText.h>
#include "TringuCam.c7"

#include "TriMesh.h"
#include "ParaSurf.h"

#include <TMath.h>

ClassImp(TringuCam)

/**************************************************************************/

void TringuCam::_init()
{
  bKeysVerbose = bMouseVerbose = false;

  //                                acc/inc | decay,    | timeout
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

  //                                  acc/inc   | decay     | timeout
  mChgParCameraRotate.SetValueParams (0.1, 0.5,   1.2, 0.4,   0.05);
  mChgParCameraRotate.SetDesireParams(0.1, 0.3,   0.1, 0.1,   0.2);

  mSpinUp.fChangeParams = &mChgParCameraRotate;
  mKeyStateMap['q'] = &mSpinUp.fIncKey;
  mKeyStateMap['e'] = &mSpinUp.fDecKey;


  // MouseAction
  mMouseAction  = MA_RayCollide;
  mRayLength    = 100;
  mActionValue  = 1;
  mActionRadius = 1;
  mActRadFract  = 0.9;

  bMouseDown              = false;
  bEnableTringDLonMouseUp = false;

  mCamFix = 0;

  mRayColl.SetCulling   (true);
  mRayColl.SetClosestHit(true);
  mRayColl.SetDestination(&mCollFaces);
  mCollVertex = -1;
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

void TringuCam::MouseDown()
{
  bMouseDown = true;

  switch (mMouseAction)
  {
    case MA_Nothing:
    {
      break;
    }
    case MA_RayCollide:
    {
      CalculateMouseRayVectors();
      GLensReadHolder _tlck(*mTringula);
      mTringula->SetRayVectors(mMouseRayPos, mMouseRayDir);
      mTringula->RayCollide();
      break;
    }
    case MA_AddField:
    {
      CalculateMouseRayVectors();
      MouseRayCollide();
      AddField(mActionValue);
      break;
    }
    case MA_SprayField:
    {
      // Everything done in time-tick.
      if (mTringula != 0 && mTringula->GetUseDispList())
      {
        mTringula->SetUseDispList(false);
        bEnableTringDLonMouseUp = true;
      }
      break;
    }
    default:
    {
      printf("Unhandled case!!!!\n");
      break;
    }
  }
}

void TringuCam::MouseUp()
{
  bMouseDown = false;
  if ( bEnableTringDLonMouseUp)
  {
    mTringula->SetUseDispList(true);
    bEnableTringDLonMouseUp = false;
  }
}

void TringuCam::CalculateMouseRayVectors()
{
  float yext = TMath::Tan(0.5*TMath::DegToRad()*mZFov)*mNearClp;
  float xext = yext*mScreenW/mScreenH;
  float xcam = xext*(2.0f*mMouseX/mScreenW - 1);
  float ycam = yext*(2.0f*mMouseY/mScreenH - 1);

  mMouseRayPos.SetXYZ(0, 0, 0);
  mCamFix->MultiplyIP(mMouseRayPos);
  mTrans.MultiplyIP(mMouseRayPos);

  mMouseRayDir.SetXYZ(mNearClp, -xcam, -ycam);
  mMouseRayDir.SetMag(1);
  mCamFix->RotateIP(mMouseRayDir);
  mTrans.RotateIP(mMouseRayDir);
}

void TringuCam::MouseRayCollide()
{
  Opcode::Ray ray;
  mMouseRayPos.GetXYZ((Float_t*)&ray.mOrig.x);
  mMouseRayDir.GetXYZ((Float_t*)&ray.mDir .x);
  ray.mDir *= mRayLength;

  Bool_t status = mRayColl.Collide(ray, *mTringula->GetMesh()->GetOPCModel());

  if (bMouseVerbose)
    printf("TringuCam::MouseRayCollide status=%d, n_faces=%d\n",
           status, mCollFaces.GetNbFaces());

  if (status && mCollFaces.GetNbFaces() > 0)
  {
    using namespace Opcode;
    const CollisionFace& cf = mCollFaces.GetFaces()[0];

    if (bMouseVerbose)
      printf("  fc=%6d  d=%7.3f  u=%7.3f v=%7.3f\n",
             cf.mFaceID, cf.mDistance, cf.mU, cf.mV);

    TringTvor& TT = * mTringula->GetMesh()->GetTTvor();
    Int_t   *t  = TT.Triangle(cf.mFaceID);
    Float_t *v0 = TT.Vertex(t[0]);
    Float_t *v1 = TT.Vertex(t[1]);
    Float_t *v2 = TT.Vertex(t[2]);

    Point e1(v1[0] - v0[0], v1[1] - v0[1], v1[2] - v0[2]);
    Point e2(v2[0] - v0[0], v2[1] - v0[1], v2[2] - v0[2]);

    mCollFace = cf;
    mCollPoint.Set(v0); mCollPoint += cf.mU*e1 + cf.mV*e2;
    Int_t ci = (cf.mU + cf.mV <= 0.5) ? 0 : (cf.mU >= cf.mV) ? 1 : 2;
    mCollVertex = t[ci];
    if (bMouseVerbose)
      printf("  x=%7.3f  y=%7.3f  z=%7.3f   closest index=%d  vertex=%d\n",
             mCollPoint.x, mCollPoint.y, mCollPoint.z, ci, mCollVertex);
  }
  else
  {
    mCollVertex = -1;
  }
}

/**************************************************************************/

void TringuCam::add_field_visit_vertex(set<Int_t>& vv, set<Int_t>& cv,
                                       Int_t v, Float_t value)
{
  if (v<0 || vv.find(v) != vv.end())
    return;

  vv.insert(v);
  TringTvor& TT = * mTringula->GetMesh()->GetTTvor();
  Opcode::Point delta(TT.Vertex(v));
  delta -= mCollPoint;
  Float_t dist = delta.Magnitude();

  if (dist > mActionRadius)
    return;

  cv.insert(v);
  Float_t full_r = mActionRadius*mActRadFract;
  if (dist <= full_r)
    mCurField->F(v) += value;
  else
    mCurField->F(v) += value*(1 - (dist - full_r)/(mActionRadius - full_r));

  const vector<TriMesh::VertexData>& VDV = mTringula->GetMesh()->RefVDataVec();
  const vector<TriMesh::EdgeData>  & EDV = mTringula->GetMesh()->RefEDataVec();

  const TriMesh::VertexData& vd = VDV[v];
  for (Int_t e = 0; e < vd.n_edges(); ++e)
  {
    const TriMesh::EdgeData& ed = EDV[vd.edge(e)];
    add_field_visit_vertex(vv, cv, ed.other_vertex(v), value);
  }
}

void TringuCam::AddField(Float_t value)
{
  set<Int_t> vv, cv; // visited/changed vertices
  add_field_visit_vertex(vv, cv, mCollVertex, value);
  if (!cv.empty())
  {
    if (mLightField == 0)
    {
      mCurField  ->PartiallyColorizeTvor(cv, true);
    }
    else
    {
      mCurField  ->PartiallyColorizeTvor(cv, false);
      mLightField->PartiallyModulateTvor(cv, true);
    }
  }
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

  { // Restore up-direction
    Float_t pos[3], hdir[3];
    mTrans.GetPos(pos);
    mTringula->GetParaSurf()->pos2hdir(pos, hdir);
    mTrans.SetBaseVec(3, hdir);
    mTrans.OrtoNorm3Column(1, 3);
    mTrans.SetBaseVecViaCross(2);
  }

  if (*mInfoTxt != 0)
    mInfoTxt->SetText
      (GForm("FWD: + %5.2f | %+5.2f | - %5.2f  ||  "
             "LFT: + %5.2f | %+5.2f | - %5.2f  || "
             " UP: + %5.2f | %+5.2f | - %5.2f",
             mFwdBck.fIncKey.fDesiredValue, mFwdBck.fValue, mFwdBck.fDecKey.fDesiredValue,
             mLftRgt.fIncKey.fDesiredValue, mLftRgt.fValue, mLftRgt.fDecKey.fDesiredValue,
             mUpDown.fIncKey.fDesiredValue, mUpDown.fValue, mUpDown.fDecKey.fDesiredValue));


  if (bMouseDown && mMouseAction == MA_SprayField)
  {
    CalculateMouseRayVectors();
    MouseRayCollide();
    AddField(dt*mActionValue);
  }
}


/**************************************************************************/
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
