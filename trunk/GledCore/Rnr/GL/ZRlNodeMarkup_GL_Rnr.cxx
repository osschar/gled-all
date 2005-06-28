// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRlNodeMarkup_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <TSystem.h>
#include <GL/gl.h>

/**************************************************************************/

void ZRlNodeMarkup_GL_Rnr::_init()
{}

/**************************************************************************/

void ZRlNodeMarkup_GL_Rnr::PreDraw(RnrDriver* rd)
{
  bExState = rd->GL()->GetMarkupNodes();
  switch(mZRlNodeMarkup->mNameRnrOp) {
  case ZRnrModBase::O_On:
    ConsiderRebuildDL(rd);
    rd->PushRnrMod(ZRlNodeMarkup::FID(), mRnrMod);
    rd->GL()->SetMarkupNodes(true);
    break;
  case ZRnrModBase::O_Off:
    rd->GL()->SetMarkupNodes(false);
    break;
  case ZRnrModBase::O_Nop:
    break;
  }
}

void ZRlNodeMarkup_GL_Rnr::Draw(RnrDriver* rd)
{
  switch(mZRlNodeMarkup->mNameRnrOp) {
  case ZRnrModBase::O_On:
    ConsiderRebuildDL(rd);
    rd->SetDefRnrMod(ZRlNodeMarkup::FID(), mRnrMod);
    rd->GL()->SetMarkupNodes(true);
    break;
  case ZRnrModBase::O_Off:
    rd->GL()->SetMarkupNodes(false);
    break;
  case ZRnrModBase::O_Nop:
    break;
  }
}

void ZRlNodeMarkup_GL_Rnr::PostDraw(RnrDriver* rd)
{
  switch(mZRlNodeMarkup->mNameRnrOp) {
  case ZRnrModBase::O_On:
    rd->PopRnrMod(ZRlNodeMarkup::FID());
    rd->GL()->SetMarkupNodes(bExState);
    break;
  case ZRnrModBase::O_Off:
    rd->GL()->SetMarkupNodes(bExState);
    break;
  case ZRnrModBase::O_Nop:
    break;
  }
}
