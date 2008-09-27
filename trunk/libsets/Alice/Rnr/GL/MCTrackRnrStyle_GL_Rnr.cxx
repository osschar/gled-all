// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "MCTrackRnrStyle_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <FL/gl.h>

/**************************************************************************/

void MCTrackRnrStyle_GL_Rnr::_init()
{}

/**************************************************************************/

void MCTrackRnrStyle_GL_Rnr::PreDraw(RnrDriver* rd)
{
  update_tring_stamp(rd);
  mTRS->calculate_abs_times();
  rd->PushRnrMod(MCTrackRnrStyle::FID(),  mRnrMod);
}

void MCTrackRnrStyle_GL_Rnr::Draw(RnrDriver* rd)
{
  update_tring_stamp(rd);
  mTRS->calculate_abs_times();
  rd->SetDefRnrMod(MCTrackRnrStyle::FID(), mRnrMod);
}

void MCTrackRnrStyle_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->PopRnrMod(MCTrackRnrStyle::FID());
}
