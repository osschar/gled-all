// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
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
  mFOV = 90; mNearClip = 0.1; mFarClip = 500;
  bLiMo2Side = false;
  mFrontMode = GL_FILL; mBackMode = GL_LINE;
  bBlend = false;
  mMSRotFac = -600; mMSMoveFac = -200;
  mCHSize = 0.03;
  bShowRPS = true;
  mBuffSize = 4096; mPickW = 5; mPickH = 5;
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
  try { set_link_or_die((ZGlass*&)mCameraBase, camerabase, LibID(), ClassID()); }
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
  try { set_link_or_die((ZGlass*&)mLookAt, lookat, LibID(), ClassID()); }
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
  try { set_link_or_die((ZGlass*&)mUpReference, upreference, LibID(), ClassID()); }
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
