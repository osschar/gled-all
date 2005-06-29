// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ITSDigRnrMod_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <FL/gl.h>

/**************************************************************************/

void ITSDigRnrMod_GL_Rnr::_init()
{}

/**************************************************************************/

void ITSDigRnrMod_GL_Rnr::PreDraw(RnrDriver* rd)
{
  ConsiderRebuildDL(rd);
  rd->PushRnrMod(ITSDigRnrMod::FID(),  mRnrMod);
}

void ITSDigRnrMod_GL_Rnr::Draw(RnrDriver* rd)
{
  ConsiderRebuildDL(rd);
  rd->SetDefRnrMod(ITSDigRnrMod::FID(), mRnrMod);
}

void ITSDigRnrMod_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->PopRnrMod(ITSDigRnrMod::FID());
}
