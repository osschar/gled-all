// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlass_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/SphereTrings.h>
#include <Ephra/Saturn.h>

/**************************************************************************/

ZGlass_GL_Rnr::ZGlass_GL_Rnr(ZGlass* d) : A_Rnr(), mGlass(d)
{
  mStampTring = 0;
  mDispList   = glGenLists(1);
  bRebuildDL         = true;
  bUseNameStack      = true;
  bSuppressNameLabel = false;
}

ZGlass_GL_Rnr::~ZGlass_GL_Rnr()
{
  glDeleteLists(mDispList, 1);
}

/**************************************************************************/

void ZGlass_GL_Rnr::PreDraw(RnrDriver* rd)
{
  if(bUseNameStack) glPushName(mGlass->mSaturnID);

  if(mGlass->mStampReqTring > mStampTring) {
    Triangulate(rd);
    mStampTring = mGlass->mTimeStamp;
    bRebuildDL  = true;
  }
}

void ZGlass_GL_Rnr::Draw(RnrDriver* rd)
{
  // cout <<"ZGlass_GL_Rnr::Draw rendering '"<< mGlass->GetName() <<"'.\n";

  if(bRebuildDL) {
    glNewList(mDispList, GL_COMPILE_AND_EXECUTE);
    Render(rd);
    glEndList();
    bRebuildDL = false;
  } else {
    glCallList(mDispList);
  }
}

void ZGlass_GL_Rnr::PostDraw(RnrDriver* rd)
{
  if(bUseNameStack) glPopName();
}

/**************************************************************************/

void ZGlass_GL_Rnr::Render(RnrDriver* rd)
{}
