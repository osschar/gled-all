// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZNode_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <Ephra/Saturn.h>

#include <FL/gl.h>

/**************************************************************************/
// Basic GL Drawing
/**************************************************************************/
// Be reasonable when redefining Pre/Post Draw in derived classes

void ZNode_GL_Rnr::build_GL_mat()
{
  int s=0;
  for(Int_t i=1; i<=3; i++) {
    for(Int_t j=1; j<=4; j++)
      mGL_Mat[s++] = mNode->mTrans(j,i);
  }
  mGL_Mat[s++] = mNode->mTrans(1,4); mGL_Mat[s++] = mNode->mTrans(2,4);
  mGL_Mat[s++] = mNode->mTrans(3,4); mGL_Mat[s++] = mNode->mTrans(4,4);
}

/**************************************************************************/

void ZNode_GL_Rnr::PreDraw(RnrDriver* rd)
{
  ZGlass_GL_Rnr::PreDraw(rd);
  if(mNode->mStampReqTrans > mStampTrans) {
    build_GL_mat();
    mStampTrans = mNode->mTimeStamp;
  }
  glPushAttrib(GL_TRANSFORM_BIT);
  glPushMatrix();
  glMultMatrixf(mGL_Mat);
  if(mNode->bUseScale) {
    glEnable(GL_NORMALIZE);
    glScalef(mNode->mSx, mNode->mSy, mNode->mSz);
  }
  rd->PushPM(mNode);
}

void ZNode_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->PopPM();
  glPopMatrix();
  glPopAttrib();
  ZGlass_GL_Rnr::PostDraw(rd);
}

