// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "PupilInfo.h"
#include "PupilInfo.c7"

ClassImp(PupilInfo)

void PupilInfo::_init()
{
  mCameraBase = 0;
  mLookAt = 0;
  mUpReference = 0;
  mUpRefAxis = 3;

  mMaxDepth = 100;
  mClearColor.rgba(0,0,0);
  mFOV = 90; mNearClip = 0.3; mFarClip = 500;
  bLiMo2Side = false;
  mFrontMode = GL_FILL; mBackMode = GL_LINE;
  bBlend = false;
  mMSRotFac = -600; mMSMoveFac = -200;
  mCHSize = 0.03;
  mBuffSize = 4096; mPickW = 5; mPickH = 5;
}

/**************************************************************************/

void PupilInfo::SetCameraBase(ZNode* camerabase)
{
  if(camerabase != 0) {
    ZTrans* t = ToPupilFrame(camerabase);
    if(t == 0)
      throw("PupilInfo::SetCameraBase camera not connected into pupil contents.");
    delete t;
  }

  mExecMutex.Lock(); 
  if(mCameraBase) mCameraBase->DecRefCount(this); 
  mCameraBase = camerabase;

  StampLink(LibID(), ClassID()); 
  if(mCameraBase) mCameraBase->IncRefCount(this); 
  mExecMutex.Unlock();
}

void PupilInfo::SetLookAt(ZNode* lookat)
{
  if(lookat != 0) {
    ZTrans* t = ToPupilFrame(lookat);
    if(t == 0)
      throw("PupilInfo::SetLookat camera not connected into pupil contents.");
    delete t;
  }

  mExecMutex.Lock(); 
  if(mLookAt) mLookAt->DecRefCount(this); 
  mLookAt = lookat;

  StampLink(LibID(), ClassID()); 
  if(mLookAt) mLookAt->IncRefCount(this); 
  mExecMutex.Unlock();
}

void PupilInfo::SetUpReference(ZNode* upreference)
{
  if(upreference != 0) {
    ZTrans* t = ToPupilFrame(upreference);
    if(t == 0)
      throw("PupilInfo::SetUpReference camera not connected into pupil contents.");
    delete t;
  }

  mExecMutex.Lock(); 
  if(mUpReference) mUpReference->DecRefCount(this); 
  mUpReference = upreference;

  StampLink(LibID(), ClassID()); 
  if(mUpReference) mUpReference->IncRefCount(this); 
  mExecMutex.Unlock();
}

/**************************************************************************/

ZTrans* PupilInfo::ToPupilFrame(ZNode* node)
{
  mListMutex.Lock();
  for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
    ZNode* pup_elm = dynamic_cast<ZNode*>(*i);
    if(pup_elm) {
      ZTrans* t = node->ToNode(pup_elm);
      if(t != 0) {
	mListMutex.Unlock();
	return t;
      }
    }
  }
  mListMutex.Unlock();
  return 0;
}
