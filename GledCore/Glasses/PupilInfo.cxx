// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "PupilInfo.h"
#include "PupilInfo.c7"
#include "ZQueen.h"

ClassImp(PupilInfo)

void PupilInfo::_init()
{
  mCameraBase = 0;
  mLookAt = 0;
  mUpReference = 0;
  mUpRefAxis = 3;

  mMaxDepth = 100;

  mWidth = 640; mHeight = 480;

  mClearColor.rgba(0,0,0);
  mFOV = 90; mNearClip = 0.01; mFarClip = 120;
  bLiMo2Side = false;
  mFrontMode = GL_FILL; mBackMode = GL_LINE;
  bBlend = false;
  mMSRotFac = 1; mMSMoveFac = 2; mMoveOM = -2; mAccelExp = 0.5;

  mCHSize = 0.03;   mTextSize = 12;

  bShowRPS = true; bRnrNames = false; bRnrTiles = true;
  mNameOffset = 0.99;
  mTextCol.rgba(1, 1, 1); mTileCol.rgba(0, 0, 0.3);

  mBuffSize = 4096; mPickR = 5; mPickDisp = 0;
}

/**************************************************************************/

void PupilInfo::SetCameraBase(ZNode* camerabase)
{
  if(camerabase != 0) {
    ZTrans* t = ToPupilFrame(camerabase);
    if(t == 0)
      throw(string("PupilInfo::SetCameraBase camera not connected into pupil contents."));
    delete t;
  }

  WriteLock();
  try { set_link_or_die((ZGlass*&)mCameraBase, camerabase, FID()); }
  catch(...) { WriteUnlock(); throw; }
  WriteUnlock();
}

void PupilInfo::SetLookAt(ZNode* lookat)
{
  if(lookat != 0) {
    ZTrans* t = ToPupilFrame(lookat);
    if(t == 0)
      throw(string("PupilInfo::SetLookat camera not connected into pupil contents."));
    delete t;
  }

  WriteLock();
  try { set_link_or_die((ZGlass*&)mLookAt, lookat, FID()); }
  catch(...) { WriteUnlock(); throw; }
  WriteUnlock();
}

void PupilInfo::SetUpReference(ZNode* upreference)
{
  if(upreference != 0) {
    ZTrans* t = ToPupilFrame(upreference);
    if(t == 0)
      throw(string("PupilInfo::SetUpReference camera not connected into pupil contents."));
    delete t;
  }

  WriteLock();
  try { set_link_or_die((ZGlass*&)mUpReference, upreference, FID()); }
  catch(...) { WriteUnlock(); throw; }
  WriteUnlock();
}

/**************************************************************************/

ZTrans* PupilInfo::ToPupilFrame(ZNode* node)
{
  mListMutex.Lock();
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
    ZNode* pup_elm = dynamic_cast<ZNode*>(*i);
    if(pup_elm) {
      if(pup_elm == node) {
	mListMutex.Unlock();
	return new ZTrans(pup_elm->RefTrans());
      } else {
	ZTrans* t = node->ToNode(pup_elm);
	if(t != 0) {
	  mListMutex.Unlock();
	  ZTrans* ret = new ZTrans(pup_elm->RefTrans());
	  *ret *= *t;
	  delete t;
	  return ret;
	}
      }
    }
  }
  mListMutex.Unlock();
  return 0;
}

/**************************************************************************/

void PupilInfo::EmitDumpImageRay(const Text_t* filename)
{
  if(mQueen && mSaturn->AcceptsRays()) {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, RayNS::RQN_user_1, mTimeStamp, FID()));
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
      (Ray::PtrCtor(this, RayNS::RQN_user_2, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}
