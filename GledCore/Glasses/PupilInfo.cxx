// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// PupilInfo
//
// Configuration data for GL viewer.
//
// Emits custom Rays: user_1 -> dump image, user_2 -> resize window

#include "PupilInfo.h"
#include "PupilInfo.c7"
#include <Glasses/ZQueen.h>

ClassImp(PupilInfo)

void PupilInfo::_init()
{
  // Override from SubShellInfo:
  mCtorLibset = "GledCore";
  mCtorName   = "Pupil";

  mCameraBase    = 0;

  mLookAt        = 0;
  mLookAtMinDist = 0.1;

  mUpReference   = 0;
  mUpRefAxis     = 3;
  bUpRefLockDir  = true;
  mUpRefMinAngle = 10;

  mMaxDepth = 100;

  mWidth = 640; mHeight = 480;

  mClearColor.rgba(0,0,0);

  mProjMode = P_Perspective;
  mFOV = 90;
  mNearClip = 0.01; mFarClip = 120;
  mYSize = 10; mHFac = 1;

  mFrontMode = GL_FILL; mBackMode = GL_LINE;
  bLiMo2Side = false;
  bBlend     = false;

  mMSRotFac = 1; mMSMoveFac = 2; mMoveOM = -2; mAccelExp = 0.5;

  mCHSize = 0.03;

  bShowRPS = true; bRnrNames = false;

  mBuffSize = 4096; mPickR = 5; mPickDisp = 0;

  mPopupDx = 200; mPopupDy = 0;
  mPopupFx = 0;   mPopupFy = -0.5;
}

/**************************************************************************/

void PupilInfo::SetCameraBase(ZNode* camerabase)
{
  static const string _eh("PupilInfo::SetCameraBase ");

  if(camerabase != 0) {
    ZTrans* t = ToPupilFrame(camerabase);
    if(t == 0)
      throw(_eh + "camera not connected into pupil contents.");
    delete t;
  }
  set_link_or_die((ZGlass*&)mCameraBase, camerabase, FID());
}

void PupilInfo::SetLookAt(ZNode* lookat)
{
  static const string _eh("PupilInfo::SetLookAt ");

  if(lookat != 0) {
    ZTrans* t = ToPupilFrame(lookat);
    if(t == 0)
      throw(_eh + "camera not connected into pupil contents.");
    delete t;
  }
  set_link_or_die((ZGlass*&)mLookAt, lookat, FID());
}

void PupilInfo::SetUpReference(ZNode* upreference)
{
  static const string _eh("PupilInfo::SetUpReference ");

  if(upreference != 0) {
    ZTrans* t = ToPupilFrame(upreference);
    if(t == 0)
      throw("camera not connected into pupil contents.");
    delete t;
  }

  set_link_or_die((ZGlass*&)mUpReference, upreference, FID());
}

/**************************************************************************/

ZTrans* PupilInfo::ToPupilFrame(ZNode* node)
{
  // Returns transformation from node->pupil reference frame.

  // This should use parental-list, like similar stuff in ZNode.

  if(node == 0) return 0;

  GMutexHolder lst_lck(mListMutex);
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
    ZNode* pup_elm = dynamic_cast<ZNode*>(*i);
    if(pup_elm) {
      if(pup_elm == node) {
	return new ZTrans(pup_elm->RefTrans());
      } else {
	ZTrans* t = node->ToNode(pup_elm);
	if(t != 0) {
	  ZTrans* ret = new ZTrans(pup_elm->RefTrans());
	  *ret *= *t;
	  delete t;
	  return ret;
	}
      }
    }
  }
  return 0;
}

ZTrans* PupilInfo::ToCameraFrame(ZNode* node)
{
  // Returns transformation from node->camera reference frame.
  // Camera's matrix is not multiplied in the result (as it is not known).

  ZTrans* n2p = ToPupilFrame(node);
  if(n2p == 0) return 0;
  
  ZNode* cam_base = mCameraBase;
  if(cam_base) {
    ZTrans* c2p = ToPupilFrame(cam_base);
    if(c2p == 0) {
      delete n2p;
      return 0;
    }
    c2p->Invert();
    *c2p *= *n2p;
    delete n2p;
    return c2p;
  }
  return n2p;
}

/**************************************************************************/

void PupilInfo::EmitDumpImageRay(const Text_t* filename)
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_dump_image, mTimeStamp, FID()));
    if(filename != 0) {
      TString fn(filename);
      TBuffer cbuff(TBuffer::kWrite);
      fn.Streamer(cbuff);
      ray->SetCustomBuffer(cbuff);
    }
    mQueen->EmitRay(ray);
  }
}

void PupilInfo::EmitImmediateRedrawRay()
{
  EmitDumpImageRay(0);
}

/**************************************************************************/

void PupilInfo::EmitResizeRay()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_resize_window, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}

/**************************************************************************/

void PupilInfo::EmitCameraHomeRay()
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_camera_home, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}
