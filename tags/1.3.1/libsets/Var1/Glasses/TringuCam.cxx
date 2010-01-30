// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TringuCam
//
//

#include "TringuCam.h"
#include "Glasses/TSPupilInfo.h"
#include "TringuRep.h"
#include "Extendio.h"
#include "Statico.h"
#include "Dynamico.h"

#include <Glasses/ZQueen.h>
#include <Glasses/ScreenText.h>
#include <Glasses/WGlWidget.h>
#include <Glasses/Eventor.h>
#include <Glasses/TimeMaker.h>
#include <Glasses/WSTube.h>

#include "TringuCam.c7"

#include "TringulaTester.h"

#include "TriMesh.h"
#include "ParaSurf.h"

#include <RnrBase/Fl_Event_Enums.h>

#include <TMath.h>

ClassImp(TringuCam);

/**************************************************************************/

void TringuCam::_init()
{
  bKeysVerbose = bMouseVerbose = false;

  //                                acc/inc | decay,    | timeout
  mChgParCameraMove.SetValueParams (1, 0.5,   1.2, 0.4,   0.05);
  mChgParCameraMove.SetDesireParams(1, 0.3,   1,   0.1,   0.2);

  mFwdBck.fChangeParams = &mChgParCameraMove;
  mFwdBck.SetMinMax(-20, 100);
  mKeyStateMap['w'] = &mFwdBck.fIncKey;
  mKeyStateMap['s'] = &mFwdBck.fDecKey;

  // Sustain mode ... in this case happens too fast.
  // Should really be attached to:
  // a) throttle controls;
  // b) use different change params - no exponential approach.
  // mFwdBck.fSustain = true;

  mLftRgt.fChangeParams = &mChgParCameraMove;
  mLftRgt.SetMinMax(-10, 10);
  mKeyStateMap['a'] = &mLftRgt.fIncKey;
  mKeyStateMap['d'] = &mLftRgt.fDecKey;

  mUpDown.fChangeParams = &mChgParCameraMove;

  mUpDown.SetMinMax(-30, 30);
  mKeyStateMap['r'] = &mUpDown.fIncKey;
  mKeyStateMap['f'] = &mUpDown.fDecKey;

  //                                  acc/inc   | decay     | timeout
  mChgParCameraRotate.SetValueParams (0.1, 0.5,   1.2, 0.4,   0.05);
  mChgParCameraRotate.SetDesireParams(0.1, 0.3,   0.1, 0.1,   0.2);

  mSpinUp.fChangeParams = &mChgParCameraRotate;
  mSpinUp.SetMinMax(-TMath::Pi(), TMath::Pi());
  mKeyStateMap['q'] = &mSpinUp.fIncKey;
  mKeyStateMap['e'] = &mSpinUp.fDecKey;


  // MouseAction
  mMouseAction  = MA_PickExtendios;
  mExpectBeta   = EB_Nothing;
  mRayLength    = 0;

  bMouseDown    = false;

  mStampInterval = 25;
  mStampCount    = 0;
  mHeight = 0;

  mRayColl.SetCulling   (true);
  mRayColl.SetClosestHit(true);
  mRayColl.SetDestination(&mCollFaces);
  mCollVertex = -1;
}

/**************************************************************************/

WSTube* TringuCam::make_tube(Statico* stato0, Statico* stato1, const TString& grad_name)
{
  WSTube* tube = new WSTube(GForm("Tube %s - %s; %s", stato0->GetName(), stato1->GetName(), grad_name.Data()));

  tube->SetTransSource(WSTube::TS_Transes);

  const HTransF& tA = stato0->ref_last_trans();
  const HTransF& tB = stato1->ref_last_trans();

  tube->RefTransA().SetFromArray(tA);
  tube->RefTransA().MoveLF(3, 2.0*stato0->get_tring_tvor()->mCtrExtBox[5]);
  tube->RefTransB().SetFromArray(tB);
  tube->RefTransB().MoveLF(3, 2.0*stato1->get_tring_tvor()->mCtrExtBox[5]);

  tube->RefVecA().SetXYZT(0, 0,  2, 1);
  tube->RefVecB().SetXYZT(0, 0, -2, 1);

  tube->SetTexture((ZImage*) mQueen->FindLensByPath(GForm("var/gradients/%s",
                                                          grad_name.Data())));

  tube->SetFat(false);
  tube->SetLineW(1.6);
  tube->SetTLevel(30);
  tube->SetPLevel(3);
  tube->SetDefWidth(0.04);
  tube->SetDefTension(1.5);
  tube->SetTexUScale(2);
  tube->SetDtexU(-0.3);

  return tube;
}

//==============================================================================

inline TringuCam::KeyInfo* TringuCam::FindKeyInfo(Int_t key)
{
  map<Int_t, KeyInfo*>::iterator i = mKeyStateMap.find(key);
  return (i != mKeyStateMap.end()) ? i->second : 0;
}

//------------------------------------------------------------------------------

Int_t TringuCam::KeyDown(Int_t key)
{
  KeyInfo* ki = FindKeyInfo(key);
  if (ki == 0) return 0;

  if (ki->fIsDown == false)
  {
    ki->fIsDown = true;

    ValueInfo            *vi =   ki->fValueInfo;
    KeyValueChangeParams &C  = * vi->fChangeParams;
    // KeyInfo*  oki = ki->fIsInc ? &vi->fDecKey : &vi->fIncKey;

    if (vi->fSustain)
    {
      if (ki->fDecayTimeout > 0)
      {
	// if pressed during timeout, set sustain-desire
	vi->fSustainSet = true;
	if (ki->fIsInc)
	{
	  if (vi->fSustainDesire < 0) vi->fSustainDesire = 0;
	  else                        vi->fSustainDesire = vi->fMaxValue;
	}
	else
	{
	  if (vi->fSustainDesire > 0) vi->fSustainDesire = 0;
	  else                        vi->fSustainDesire = vi->fMinValue;
	}
      }
      else
      {
	// else ... hmmh, set key desire, for no apparent reason.
	ki->fDesiredValue = ki->fIsInc ? vi->fMaxValue : - vi->fMinValue;
      }      
    }
    else
    {
      if (ki->fDecayTimeout > 0)
      {
	// if pressed during timeout: inc + delta_factor
	ki->fDesiredValue += C.fDesireIncDeltaFactor * ki->fDesiredValue;
	ki->fDesiredValue += C.fDesireIncStep;
      }
      else
      {
	// else round-up.
	ki->fDesiredValue = TMath::Ceil(ki->fDesiredValue + 0.001);
      }
    }

    // Make sure we do not exceed limits.
    if (ki->fIsInc)
    {
      if (ki->fDesiredValue > vi->fMaxValue)
        ki->fDesiredValue = vi->fMaxValue;
    }
    else
    {
      if (ki->fDesiredValue > -vi->fMinValue)
        ki->fDesiredValue = - vi->fMinValue;
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
      if (vi->fSustain)
      {
	if (!vi->fSustainSet)
	  vi->fSustainDesire = vi->fValue;
	vi->fSustainSet = false;
      }
      else
      {
	vi->fDecayTimeout = C.fValueDecayTimeout;
      }
    }
  }

  return 1;
}

//==============================================================================

void TringuCam::MouseDown(A_Rnr::Fl_Event& ev)
{
  static const Exc_t _eh("TringuCam::MouseDown ");

  bMouseDown = true;

  switch (mMouseAction)
  {
    case MA_Nothing:
    {
      break;
    }
    case MA_RayCollide:
    {
      TringulaTester *tt = dynamic_cast<TringulaTester*>(mTringuRep->GetElementByName("TringulaTester"));
      if (tt)
      {
	CalculateMouseRayVectors();
	tt->SetRayVectors(mMouseRayPos, mMouseRayDir);
	tt->RayCollideTerrain();
      }
      else
      {
	ISwarn(_eh + "can not get TringulaTester as child of TringuRep.");
      }
      break;
    }
    case MA_AddField:
    {
      CalculateMouseRayVectors();
      MouseRayCollide();
      mTringuRep->AddField(mCollPoint, mCollVertex, 1.0f);
      break;
    }
    case MA_SprayField:
    {
      mTringuRep->BeginSprayField();
      // Field is added in time-tick.
      break;
    }
    case MA_PickExtendios:
    {
      CalculateMouseRayVectors();
      Opcode::Ray ray(&mMouseRayPos[0], &mMouseRayDir[0]);
      Float_t     rng = mTringula->RayCollideClosestHit(ray, true);
      Extendio	 *ext = mTringula->PickExtendios(ray, rng);

      if (bMouseVerbose)
        printf("TringuCam::MouseDown picked %s, state=0x%x\n",
               ext ? ext->GetName() : "<none>", ev.fState);

      if (mExpectBeta == EB_ConnectStaticos)
      {
	Statico* stato = dynamic_cast<Statico*>(ext);
	if (stato)
	{
	  Statico* beta = dynamic_cast<Statico*>(*mPrepBeta);
	  if (beta) {
	    WSTube* tube = make_tube(beta, stato, mGradName);
            mQueen->CheckIn(tube);
            {
              GLensWriteHolder _wlck(*mTringula);
              mTringula->GetTubes()->Add(tube);
            }
            tube->AnimateConnect();
	  }
	}
        mExpectBeta = EB_Nothing;
        SetPrepBeta(0);
      }
      else
      {
	mPupilInfo->SelectExtendio(ext, ev.fState & FL_CTRL);
      }

      break;
    }
    case MA_NewLandMark:
    {
      printf ("Sucking new landmark\n");

      CalculateMouseRayVectors();
      MouseRayCollide();

      if (mCollVertex != -1)
      {
	TriMesh *mesh = dynamic_cast<TriMesh*>(mQueen->FindLensByPath("var/meshes/LandMark"));
	GLensReadHolder _tlck(*mTringula);
	mTringula->AddLandMark(mesh, mCollPoint);
      }

      mMouseAction = mPrevAction;
      Stamp(FID());
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
 
  if (mMouseAction == MA_SprayField)
  {
    mTringuRep->EndSprayField();
  }
}

//==============================================================================

void TringuCam::CalculateMouseRayVectors()
{
  mPupilInfo->TransformMouseRayVectors(*mTringula, mMouseRayPos, mMouseRayDir);
}

void TringuCam::MouseRayCollide()
{
  // Collide current mouse-ray with tringula terrain.

  Opcode::Ray ray;
  mMouseRayPos.GetXYZ(ray.mOrig);
  mMouseRayDir.GetXYZ(ray.mDir);

  mRayColl.SetMaxDist(mRayLength > 0 ? mRayLength : Opcode::MAX_FLOAT);

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

//==============================================================================

void TringuCam::TimeTick(Double_t t, Double_t dt)
{
  // Handle currently pressed keys.
  //
  // Mouse-2 camera rotation is handled in GL_Rnr.

  mFwdBck.TimeTick(dt);
  if (mFwdBck.fValue) MoveLF(1, dt*mFwdBck.fValue);

  mLftRgt.TimeTick(dt);
  if (mLftRgt.fValue) MoveLF(2, dt*mLftRgt.fValue);

  mUpDown.TimeTick(dt);
  if (mUpDown.fValue) mHeight += dt*mUpDown.fValue;
  if (mHeight > mTringula->GetMaxCameraH())
    mHeight = mTringula->GetMaxCameraH();

  mSpinUp.TimeTick(dt);
  if (mSpinUp.fValue) RotateLF(1, 2, dt*mSpinUp.fValue);

  { // Restore up-direction and height
    Float_t pos[3], fgh[3], hdir[3];
    mTrans.GetPos(pos);
    mTringula->GetParaSurf()->pos2fgh (pos, fgh);
    mTringula->GetParaSurf()->fgh2hdir(fgh, hdir);
    // printf("pos(%.2f, %.2f, %.2f) | fgh(%.2f, %.2f, %.2f) | hdir(%.2f, %.2f, %.2f)\n",
    //        pos[0], pos[1], pos[2], fgh[0], fgh[1], fgh[2], hdir[0],  hdir[2],  hdir[2]);

    mTrans.SetBaseVec(3, hdir);
    mTrans.OrtoNorm3Column(1, 3);
    mTrans.SetBaseVecViaCross(2);
    mTrans.MoveLF(3, mHeight - fgh[2]);
  }

  if (*mInfoTxt != 0)
    mInfoTxt->SetText
      (GForm("FWD: + %5.2f | %+5.2f | - %5.2f  ||  "
             "LFT: + %5.2f | %+5.2f | - %5.2f  || "
             " UP: + %5.2f | %+5.2f | - %5.2f",
             mFwdBck.fIncKey.fDesiredValue, mFwdBck.fValue, mFwdBck.fDecKey.fDesiredValue,
             mLftRgt.fIncKey.fDesiredValue, mLftRgt.fValue, mLftRgt.fDecKey.fDesiredValue,
             mUpDown.fIncKey.fDesiredValue, mUpDown.fValue, mUpDown.fDecKey.fDesiredValue));


  // This should be handled elsewhere, elsewise.
  if (bMouseDown && mMouseAction == MA_SprayField && mTringuRep->GetField() != 0)
  {
    CalculateMouseRayVectors();
    MouseRayCollide();
    mTringuRep->AddField(mCollPoint, mCollVertex, dt);
  }

  if (mStampInterval && --mStampCount < 0)
  {
    mStampCount = mStampInterval;
    Stamp();
  }
}


/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
// TringuCam::ValueInfo
/**************************************************************************/

void
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

void
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

void
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

void
TringuCam::ValueInfo::DecayTimeoutOrValue(Float_t& timeout, Float_t& value,
                                          Float_t  dt,      Float_t  decay,
                                          Float_t  delta_fac)
{
  if (timeout != 0)
    DecayValue(timeout, dt);
  else if (value != 0)
    DecayValue(value, decay, delta_fac);
}

void
TringuCam::ValueInfo::ApproachValue(Float_t& value, Float_t desire,
				    Float_t inc_step, Float_t inc_delta_fac,
				    Float_t dec_step, Float_t dec_delta_fac)
{
  if (value < desire)
    IncValue(value, desire, inc_step, inc_delta_fac);
  else if (value > desire)
    DecValue(value, -desire, dec_step, dec_delta_fac);
  // !!! the -desire is hack, together with -desire in DecValue
  // need better encapsulation of all this shit.
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

  if (fSustain)
  {
    if (fIncKey.fIsDown && fDecKey.fIsDown)
    {
      // do nothing ... it seems
    }
    else if ( ! fIncKey.fIsDown && ! fDecKey.fIsDown)
    {
      ApproachValue(fValue, fSustainDesire, val_acc, val_acc_dfac, val_decay, val_decay_dfac);

      DecayTimeoutOrValue(fIncKey.fDecayTimeout, fIncKey.fDesiredValue,
			  dt, des_decay, des_decay_dfac);
      DecayTimeoutOrValue(fDecKey.fDecayTimeout, fDecKey.fDesiredValue,
			  dt, des_decay, des_decay_dfac);
    }
    else
    {
      if (fIncKey.fIsDown)
      {
	ApproachValue(fValue, fMaxValue, val_acc, val_acc_dfac, val_decay, val_decay_dfac);

	DecayTimeoutOrValue(fDecKey.fDecayTimeout, fDecKey.fDesiredValue,
			    dt, des_decay, des_decay_dfac);
      }
      else
      {
	ApproachValue(fValue, fMinValue, val_acc, val_acc_dfac, val_decay, val_decay_dfac);

	DecayTimeoutOrValue(fIncKey.fDecayTimeout, fIncKey.fDesiredValue,
			    dt, des_decay, des_decay_dfac);
      }
    }

  }
  else
  {
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
}

//==============================================================================

void TringuCam::ExtendioDetails(Extendio* ext)
{
  // Should show detailed UI, just Dump data for now,

  if (ext)
    ext->Dump();
}

void TringuCam::ExtendioExplode(Extendio* ext)
{
  // Terminate a dyno.
  // Called in a deteached thread.
  //
  // This is all wrong ... but want to try this.
  // This is much better now.

  static const Exc_t _eh("TringuCam::ExtendioExplode ");

  printf("Extendio '%s' exploding at your command!\n", ext->GetName());

  ext->TakeDamage(1000);
}

//==============================================================================

void TringuCam::PrepConnectStatos(Statico* stato, Int_t id, const TString& grad)
{
  // printf("TringuCam::PrepConnectStatos id=%d grad='%s'\n", id, grad.Data());

  SetPrepBeta(stato);
  mExpectBeta = EB_ConnectStaticos;
  mGradName = grad;
}

//==============================================================================

void TringuCam::Suspend()
{
  mEventor->Stop();
  mPupilInfo->SetAutoRedraw(true);
}

void TringuCam::Resume()
{
  mTimeMaker->SetLastTOK(false);
  mPupilInfo->SetAutoRedraw(false);
  mEventor->Start();
}

//==============================================================================

namespace {

const char* help_text =
  "\n"
  "================================================================================\n"
  "HELP FOR GREED-WORLD TECH-DEMO\n"
  "================================================================================\n"
  "Movement keys\n"
  "--------------------------------------------------------------------------------\n"
  "w, s - move forward / backward\n"
  "q, e - turn left / right\n"
  "a, d - move left / right\n"
  "r, f - move up / down\n"
  "Press a key several times to move faster.\n"
  "================================================================================\n"
  "Mouse buttons\n"
  "--------------------------------------------------------------------------------\n"
  "Mouse-1 - select objects\n"
  "Mouse-2 - rotate up/down, left/right\n"
  "Number-widgets can be manipulated by Mouse-drag (M2 - x10, M3 - x100)\n"
  "================================================================================\n"
  "Camera / screen controls\n"
  "--------------------------------------------------------------------------------\n"
  "Home     - look horizontally / return camera home\n"
  "Ctrl-F12 - fullscreen mode\n"
  "Ctrl-`   - go to native gled event handling (or switch back)\n"
  "--------------------------------------------------------------------------------\n";
}

void TringuCam::Help()
{
  fputs(help_text, stdout);
}

//==============================================================================

#include <Glasses/ZVector.h>

void TringuCam::RandomStatico()
{
  static const Exc_t _eh("TringuCam::RandomStatico ");

  ZVector *statos = dynamic_cast<ZVector*>
    (mQueen->FindLensByPath("var/meshes/rndstatos"));

  if (!statos)
    throw _eh + "stato-mesh vector not found.";

  GLensReadHolder _tlck(*mTringula);
  Statico* s = mTringula->RandomStatico(statos);
  if (s == 0)
    throw _eh + "placement seems to have failed.";
  else
    printf("%screated '%s'\n", _eh.Data(), s->GetName());
}

void TringuCam::MakeLandMark()
{
  if (mMouseAction != MA_NewLandMark)
    mPrevAction = mMouseAction;
  mMouseAction = MA_NewLandMark;

  Stamp(FID());
}
