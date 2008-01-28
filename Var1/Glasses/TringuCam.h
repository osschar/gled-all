// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringuCam_H
#define Var1_TringuCam_H

#include <Glasses/ZNode.h>
#include <Glasses/Tringula.h>
#include <Stones/TimeMakerClient.h>
#include <Gled/GTime.h>

#include <Opcode/Opcode.h>

class TriMeshField;
class TriMeshLightField;

class ScreenText;

class PupilInfo;
class Eventor;
class TimeMaker;

class TringuCam : public ZNode, public TimeMakerClient
{
  MAC_RNR_FRIENDS(TringuCam);

protected:
  //=================================================================
  // Top-level-stuff
  //=================================================================

  Bool_t        bKeysVerbose;  // X{GS} 7 Bool(-join=>1)
  Bool_t        bMouseVerbose; // X{GS} 7 Bool()

  //=================================================================
  // Key-handling stuff.
  //=================================================================

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

  ZLink<ScreenText>  mInfoTxt; // X{GS} L{}


  //=================================================================
  // Mouse-handling stuff.
  //=================================================================

public:
  // On mouse-1 we do:
  enum MouseAction_e { MA_Nothing, MA_RayCollide, MA_AddField, MA_SprayField,
                       MA_AddSource };

protected:
  MouseAction_e mMouseAction;   // X{GS} 7 PhonyEnum()
  Float_t       mRayLength;     // X{GS} 7 Value(-range=>[   0, 1000, 1,100])
  Float_t       mActionValue;   // X{GS} 7 Value(-range=>[-100,  100, 1,100], -tooltip=>"value to add to field/source", -join=>1)
  Float_t       mActionRadius;  // X{GS} 7 Value(-range=>[   0,  100, 1,100], -tooltip=>"distance of vertices for which to add field/source")
  Float_t       mActRadFract;   // X{GS} 7 Value(-range=>[   0,    1, 1,1000])

  Bool_t        bMouseDown;
  Bool_t        bEnableTringDLonMouseUp;

  // Missing parameters for fall-off of value with distance from coll-point.

  //=================================================================
  // Other stuff.
  //=================================================================

  ZLink<Tringula>          mTringula;    // X{GS} L{A}
  ZLink<TriMeshField>      mCurField;    // X{GS} L{A}
  ZLink<TriMeshLightField> mLightField;  // X{GS} L{A}

  ZLink<PupilInfo>         mPupilInfo;   // X{GS} L{A}
  ZLink<Eventor>           mEventor;     // X{GS} L{A}
  ZLink<TimeMaker>         mTimeMaker;   // X{GS} L{A}

  // Render-driver and mouse state fed in via TringuCam_GL_Rnr
  // Draw() and Handle().

  ZTrans* mCamFix;              //! X{G}
  Int_t   mScreenW, mScreenH;   //!
  Float_t mNearClp, mFarClp;    //!
  Float_t mZFov;                //!
  Int_t   mMouseX,  mMouseY;    //!
  Int_t   mMPushX,  mMPushY;    //!
  Int_t   mMDrgDX,  mMDrgDY;    //!

  Int_t     mStampInterval;     //  X{GS} 7 Value(-range=>[0,1000])
  Int_t     mStampCount;        //!
  Double_t  mHeight;            //  Height above tringula. X{GS} 7 Value(-range=>[-1e5, 1e5, 1, 100])

  TVector3  mMouseRayPos;
  TVector3  mMouseRayDir;

  Opcode::RayCollider    mRayColl;    //!
  Opcode::CollisionFaces mCollFaces;  //!
  Opcode::CollisionFace  mCollFace;   //!
  Opcode::Point          mCollPoint;  //!
  Int_t                  mCollVertex; //! Closest vertex, -1 if no collision.

public:
  TringuCam(const Text_t* n="TringuCam", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  KeyInfo* FindKeyInfo(Int_t key);
  Int_t KeyDown(Int_t key);
  Int_t KeyUp(Int_t key);

  void MouseDown();
  void MouseUp();

  void CalculateMouseRayVectors();
  void MouseRayCollide();

  void add_field_visit_vertex(set<Int_t>& vv, set<Int_t>& cv, Int_t v, Float_t value);
  void AddField(Float_t val);

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

  void Suspend(); // X{E}
  void Resume();  // X{E}

#include "TringuCam.h7"
  ClassDef(TringuCam, 1)
}; // endclass TringuCam


#endif
