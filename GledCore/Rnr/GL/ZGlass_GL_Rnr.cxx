// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlass_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/SphereTrings.h>
#include <Ephra/Saturn.h>

#include <FL/gl.h>

void ZGlass_GL_Rnr::PreDraw(RnrDriver* rd)
{
  // mGlass->ExecMutex.Lock();
  glPushName(mGlass->mSaturnID);

  if(mGlass->mStampReqTring > mStampTring)
    Triangulate();
}

void ZGlass_GL_Rnr::Draw(RnrDriver* rd)
{
  // cout <<"Rendering "<< mGlass->GetName() <<endl;
  // Should bracket the code with:
  if(rd->GetRnrPureGlasses()) {
    SphereTrings::EnableGL(0);
    SphereTrings::DrawAndDisableGL(0);
  }
  // } 
  // Make sure to do so in your GL_Rnrs or the picking won't work
}

void ZGlass_GL_Rnr::PostDraw(RnrDriver* rd)
{
  glPopName();
  // mGlass->ExecMutex.Unlock();
}

void ZGlass_GL_Rnr::Triangulate()
{
  // Do whatever ... mark sth for the Rnr itself to regenerate the DL
  mStampTring = mGlass->mTimeStamp;
}
