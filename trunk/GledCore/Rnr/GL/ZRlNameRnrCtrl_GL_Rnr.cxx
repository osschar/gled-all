// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRlNameRnrCtrl_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <TSystem.h>
#include <FL/gl.h>

/**************************************************************************/

void ZRlNameRnrCtrl_GL_Rnr::_init()
{
}

/**************************************************************************/

void ZRlNameRnrCtrl_GL_Rnr::PreDraw(RnrDriver* rd)
{
  bExState = rd->GetRnrNames();
  switch(mZRlNameRnrCtrl->mNameRnrOp) {
  case ZGlStateBase::GSO_On:
    rd->PushRnrMod(ZRlNameRnrCtrl::FID(), mZRlNameRnrCtrl);
    rd->SetRnrNames(true);
    break;
  case ZGlStateBase::GSO_Off:
    rd->SetRnrNames(false);
    break;
  case ZGlStateBase::GSO_Nop:
    break;
  }
}

void ZRlNameRnrCtrl_GL_Rnr::Draw(RnrDriver* rd)
{
  switch(mZRlNameRnrCtrl->mNameRnrOp) {
  case ZGlStateBase::GSO_On:
    rd->SetDefRnrMod(ZRlNameRnrCtrl::FID(), mZRlNameRnrCtrl);
    rd->SetRnrNames(true);
    break;
  case ZGlStateBase::GSO_Off:
    rd->SetRnrNames(false);
    break;
  case ZGlStateBase::GSO_Nop:
    break;
  }
  
}

void ZRlNameRnrCtrl_GL_Rnr::PostDraw(RnrDriver* rd)
{
  switch(mZRlNameRnrCtrl->mNameRnrOp) {
  case ZGlStateBase::GSO_On:
    rd->PopRnrMod(ZRlNameRnrCtrl::FID());
    rd->SetRnrNames(bExState);
    break;
  case ZGlStateBase::GSO_Off:
    rd->SetRnrNames(bExState);
    break;
  case ZGlStateBase::GSO_Nop:
    break;
  }
 
}
