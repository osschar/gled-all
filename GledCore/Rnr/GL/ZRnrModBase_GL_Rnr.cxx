// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRnrModBase_GL_Rnr.h"
#include <FL/gl.h>

/**************************************************************************/

void ZRnrModBase_GL_Rnr::_init()
{
  // In sub-classes might want to set A_Rnr::bOnePerRnrDriver to true.

  mRnrMod = new RnrDriver::RnrMod(mZRnrModBase, this);
  mRebuildRnrCount = 0;
}

ZRnrModBase_GL_Rnr::~ZRnrModBase_GL_Rnr()
{
  delete mRnrMod;
}

/**************************************************************************/

void ZRnrModBase_GL_Rnr::PreDraw(RnrDriver* rd)
{
  // Override Tring stamping so it is published via RnrMods.
  // Ignore self-triangulation and name-stack.
  // Must persist over one rendering (if this RnrMod is used several times).

  if(mRebuildRnrCount != rd->GetRnrCount()) {

    if(mGlass->GetStampReqTring() > mStampTring) {
      mStampTring         = mGlass->GetTimeStamp();
      mRnrMod->bRebuildDL = true;
    } else {
      mRnrMod->bRebuildDL = false;
    }

    mRebuildRnrCount = rd->GetRnrCount();
  }
}

void ZRnrModBase_GL_Rnr::Draw(RnrDriver* rd)
{
  // Nothing to be done here. Suppress ZGlass implementation.
}

void ZRnrModBase_GL_Rnr::PostDraw(RnrDriver* rd)
{
  // Nothing to be done here. Suppress ZGlass implementation.
}
