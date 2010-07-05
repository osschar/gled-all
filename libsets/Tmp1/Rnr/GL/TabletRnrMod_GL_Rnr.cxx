// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletRnrMod_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>

// #include <GL/glew.h>

#define PARENT ZRnrModBase_GL_Rnr

//==============================================================================

void TabletRnrMod_GL_Rnr::_init()
{}

TabletRnrMod_GL_Rnr::TabletRnrMod_GL_Rnr(TabletRnrMod* idol) :
  ZRnrModBase_GL_Rnr(idol),
  mTabletRnrMod(idol)
{
  _init();
}

TabletRnrMod_GL_Rnr::~TabletRnrMod_GL_Rnr()
{}

//==============================================================================

void TabletRnrMod_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);
  update_tring_stamp(rd);
  rd->PushRnrMod(TabletRnrMod::FID(), mRnrMod);
}

void TabletRnrMod_GL_Rnr::Draw(RnrDriver* rd)
{
  update_tring_stamp(rd);
  rd->SetDefRnrMod(TabletRnrMod::FID(), mRnrMod);
}

void TabletRnrMod_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->PopRnrMod(TabletRnrMod::FID());
  PARENT::PostDraw(rd);
}
