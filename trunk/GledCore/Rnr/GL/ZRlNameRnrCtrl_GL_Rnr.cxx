// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRlNameRnrCtrl_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <TSystem.h>
#include <GL/gl.h>

/**************************************************************************/

void ZRlNameRnrCtrl_GL_Rnr::_init()
{}

/**************************************************************************/

void ZRlNameRnrCtrl_GL_Rnr::PreDraw(RnrDriver* rd)
{
  bExState = rd->GetMarkupNodes();
  switch(mZRlNameRnrCtrl->mNameRnrOp) {
  case ZRnrModBase::O_On:
    ConsiderRebuildDL(rd);
    rd->PushRnrMod(ZRlNameRnrCtrl::FID(), mRnrMod);
    rd->SetMarkupNodes(true);
    break;
  case ZRnrModBase::O_Off:
    rd->SetMarkupNodes(false);
    break;
  case ZRnrModBase::O_Nop:
    break;
  }
}

void ZRlNameRnrCtrl_GL_Rnr::Draw(RnrDriver* rd)
{
  switch(mZRlNameRnrCtrl->mNameRnrOp) {
  case ZRnrModBase::O_On:
    ConsiderRebuildDL(rd);
    rd->SetDefRnrMod(ZRlNameRnrCtrl::FID(), mRnrMod);
    rd->SetMarkupNodes(true);
    break;
  case ZRnrModBase::O_Off:
    rd->SetMarkupNodes(false);
    break;
  case ZRnrModBase::O_Nop:
    break;
  }
}

void ZRlNameRnrCtrl_GL_Rnr::PostDraw(RnrDriver* rd)
{
  switch(mZRlNameRnrCtrl->mNameRnrOp) {
  case ZRnrModBase::O_On:
    rd->PopRnrMod(ZRlNameRnrCtrl::FID());
    rd->SetMarkupNodes(bExState);
    break;
  case ZRnrModBase::O_Off:
    rd->SetMarkupNodes(bExState);
    break;
  case ZRnrModBase::O_Nop:
    break;
  }
}
