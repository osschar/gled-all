// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRlNameRnrCtrl_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <TSystem.h>
#include <FL/gl.h>

/**************************************************************************/

void ZRlNameRnrCtrl_GL_Rnr::_init()
{
  // A_Rnr
  bOnePerRnrDriver = true;
}

/**************************************************************************/

void ZRlNameRnrCtrl_GL_Rnr::PreDraw(RnrDriver* rd)
{
  bExState = rd->GetRnrNames();
  switch(mZRlNameRnrCtrl->mNameRnrOp) {
  case ZRnrModBase::O_On:
    rd->PushRnrMod(ZRlNameRnrCtrl::FID(), mRnrMod);
    rd->SetRnrNames(true);
    break;
  case ZRnrModBase::O_Off:
    rd->SetRnrNames(false);
    break;
  case ZRnrModBase::O_Nop:
    break;
  }
}

void ZRlNameRnrCtrl_GL_Rnr::Draw(RnrDriver* rd)
{
  switch(mZRlNameRnrCtrl->mNameRnrOp) {
  case ZRnrModBase::O_On:
    rd->SetDefRnrMod(ZRlNameRnrCtrl::FID(), mRnrMod);
    rd->SetRnrNames(true);
    break;
  case ZRnrModBase::O_Off:
    rd->SetRnrNames(false);
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
    rd->SetRnrNames(bExState);
    break;
  case ZRnrModBase::O_Off:
    rd->SetRnrNames(bExState);
    break;
  case ZRnrModBase::O_Nop:
    break;
  }
}
