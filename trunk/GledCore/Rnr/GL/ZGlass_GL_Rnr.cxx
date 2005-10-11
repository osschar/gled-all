// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlass_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/SphereTrings.h>
#include <Ephra/Saturn.h>

/**************************************************************************/

ZGlass_GL_Rnr::ZGlass_GL_Rnr(ZGlass* d) : A_Rnr(), mGlass(d)
{
  mStampTring = 0;
  mDispList   = glGenLists(1);
  bRebuildDL         = true;

  bUsesSubPicking    = false;
  bSuppressNameLabel = false;
}

ZGlass_GL_Rnr::~ZGlass_GL_Rnr()
{
  glDeleteLists(mDispList, 1);
}

/**************************************************************************/

void ZGlass_GL_Rnr::PreDraw(RnrDriver* rd)
{
  if(mGlass->bUseNameStack) rd->GL()->PushName(this);

  if(mGlass->mStampReqTring > mStampTring) {
    Triangulate(rd);
    mStampTring = mGlass->mTimeStamp;
    bRebuildDL  = true;
  }
}

void ZGlass_GL_Rnr::Draw(RnrDriver* rd)
{
  // cout <<"ZGlass_GL_Rnr::Draw rendering '"<< mGlass->GetName() <<"'.\n";

  if(mGlass->bUseDispList) {
    GLRnrDriver* glrd = rd->GL();
    if(bUsesSubPicking && glrd->PickingP()) {
      Render(rd);
    }
    else if(bRebuildDL) {
      if(glrd->GetInDLRebuild()) {
	Render(rd);
      } else {
	glrd->SetInDLRebuild(true);
	glNewList(mDispList, GL_COMPILE_AND_EXECUTE);
	Render(rd);
	glEndList();
	glrd->SetInDLRebuild(false);
	bRebuildDL = false;
      }
    }
    else {
      glCallList(mDispList);
    }
  } else {
    Render(rd);
  }
}

void ZGlass_GL_Rnr::PostDraw(RnrDriver* rd)
{
  if(mGlass->bUseNameStack) rd->GL()->PopName();
}

/**************************************************************************/

void ZGlass_GL_Rnr::Render(RnrDriver* rd)
{}

/**************************************************************************/

void ZGlass_GL_Rnr::Redraw(RnrDriver* rd)
{
  bRebuildDL = true;
  rd->GL()->SetRedraw(true);
}

/**************************************************************************/

void ZGlass_GL_Rnr::obtain_rnrmod(RnrDriver* rd, RnrModStore& rms)
{
  RnrMod* rm = rd->GetRnrMod(rms.fFid);
  if(rm != rms.fRnrMod || rm->fTringTS > rms.fTringTS) {
    bRebuildDL   = true;
    rms.fRnrMod  = rm;
    rms.fTringTS = rm->fTringTS;
  }
}
