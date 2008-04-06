// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRlNameStack_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <GL/gl.h>

#define PARENT ZRnrModBase_GL_Rnr

/**************************************************************************/

void ZRlNameStack_GL_Rnr::_init()
{}

/**************************************************************************/

void ZRlNameStack_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);
  bExState = rd->GL()->GetDoPickOps();
  switch(mZRlNameStack->mNameStackOp) {
  case ZRnrModBase::O_On:
    rd->PushRnrMod(ZRlNameStack::FID(), mRnrMod);
    rd->GL()->SetDoPickOps(true);
    break;
  case ZRnrModBase::O_Off:
    rd->GL()->SetDoPickOps(false);
    if(mZRlNameStack->bClearStack) rd->GL()->ClearNameStack();
    break;
  case ZRnrModBase::O_Nop:
    break;
  }
}

void ZRlNameStack_GL_Rnr::Draw(RnrDriver* rd)
{
  switch(mZRlNameStack->mNameStackOp) {
  case ZRnrModBase::O_On:
    rd->SetDefRnrMod(ZRlNameStack::FID(), mRnrMod);
    rd->GL()->SetDoPickOps(true);
    break;
  case ZRnrModBase::O_Off:
    rd->GL()->SetDoPickOps(false);
    break;
  case ZRnrModBase::O_Nop:
    break;
  }
  if(mZRlNameStack->bClearStack)   rd->GL()->ClearNameStack();
  if(mZRlNameStack->bRestoreStack) rd->GL()->RestoreNameStack();
}

void ZRlNameStack_GL_Rnr::PostDraw(RnrDriver* rd)
{
  switch(mZRlNameStack->mNameStackOp) {
  case ZRnrModBase::O_On:
    rd->PopRnrMod(ZRlNameStack::FID());
    rd->GL()->SetDoPickOps(bExState);
    break;
  case ZRnrModBase::O_Off:
    rd->GL()->SetDoPickOps(bExState);
    if(mZRlNameStack->bRestoreStack) rd->GL()->RestoreNameStack();
    break;
  case ZRnrModBase::O_Nop:
    break;
  }
  PARENT::PostDraw(rd);
}
