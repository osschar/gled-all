// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "PRSBase_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <FL/gl.h>

/**************************************************************************/

void PRSBase_GL_Rnr::_init()
{}

/**************************************************************************/

void PRSBase_GL_Rnr::PreDraw(RnrDriver* rd)
{
  update_tring_stamp(rd);
  rd->PushRnrMod(PRSBase::FID(),  mRnrMod);
}

void PRSBase_GL_Rnr::Draw(RnrDriver* rd)
{
  update_tring_stamp(rd);
  rd->SetDefRnrMod(PRSBase::FID(), mRnrMod);
}

void PRSBase_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->PopRnrMod(PRSBase::FID());
}
