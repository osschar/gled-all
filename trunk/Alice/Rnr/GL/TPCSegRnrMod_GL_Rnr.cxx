// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TPCSegRnrMod_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <FL/gl.h>

#define PARENT ZRnrModBase_GL_Rnr

/**************************************************************************/

void TPCSegRnrMod_GL_Rnr::_init()
{}

/**************************************************************************/

void TPCSegRnrMod_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);
  update_tring_stamp(rd);
  rd->PushRnrMod(TPCSegRnrMod::FID(),  mRnrMod);
}

void TPCSegRnrMod_GL_Rnr::Draw(RnrDriver* rd)
{
  update_tring_stamp(rd);
  rd->SetDefRnrMod(TPCSegRnrMod::FID(), mRnrMod);
}

void TPCSegRnrMod_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->PopRnrMod(TPCSegRnrMod::FID());
  PARENT::PostDraw(rd);
}
