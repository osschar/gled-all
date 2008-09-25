// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "PupilInfo.h"

ClassImp(PupilInfo)

void PupilInfo::_init()
{
  mMaxDepth = 100;
  mClearColor.rgba(0,0,0);
  mFOV = 90; mNearClip = 0.3; mFarClip = 500;
  bLiMo2Side = false;
  //mFrontMode = PM_Fill; mBackMode = PM_Line;
  mFrontMode = GL_FILL; mBackMode = GL_LINE;
  bBlend = false;
  mMSRotFac = -600; mMSMoveFac = -200;
  mCHSize = 0.03;
  mBuffSize = 4096; mPickW = 5; mPickH = 5;
}

#include "PupilInfo.c7"
