// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
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
  for(UCIndex_t i=1; i<=3; i++) {
    for(UCIndex_t j=1; j<=4; j++)
      mGL_Mat[s++] = mNode->mTrans(j,i);
  }
  mGL_Mat[s++] = mNode->mTrans(1u,4u); mGL_Mat[s++] = mNode->mTrans(2u,4u);
  mGL_Mat[s++] = mNode->mTrans(3u,4u); mGL_Mat[s++] = mNode->mTrans(4u,4u);
}

/**************************************************************************/

void ZNode_GL_Rnr::PreDraw(RnrDriver* rd)
{
  ZGlass_GL_Rnr::PreDraw(rd);
  if(mNode->mStampReqTrans > mStampTrans) {
    build_GL_mat();
    mStampTrans = mNode->mStampReqTrans;
  }
  glPushMatrix();
  glMultMatrixf(mGL_Mat);
  rd->PushPM(mNode);
}

void ZNode_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->PopPM();
  glPopMatrix();
  ZGlass_GL_Rnr::PostDraw(rd);
}

