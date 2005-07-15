// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TOFDigRnrMod_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <GL/gl.h>

/**************************************************************************/

void TOFDigRnrMod_GL_Rnr::_init()
{}

/**************************************************************************/

void TOFDigRnrMod_GL_Rnr::PreDraw(RnrDriver* rd)
{
  update_tring_stamp(rd);
  rd->PushRnrMod(TOFDigRnrMod::FID(),  mRnrMod);
}

void TOFDigRnrMod_GL_Rnr::Draw(RnrDriver* rd)
{
  update_tring_stamp(rd);
  rd->SetDefRnrMod(TOFDigRnrMod::FID(), mRnrMod);
}

void TOFDigRnrMod_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->PopRnrMod(TOFDigRnrMod::FID());
}
