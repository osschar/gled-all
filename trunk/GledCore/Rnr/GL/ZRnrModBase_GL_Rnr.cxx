// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRnrModBase_GL_Rnr.h"
#include <FL/gl.h>

/**************************************************************************/

void ZRnrModBase_GL_Rnr::_init()
{
  // A_Rnr
  bOnePerRnrDriver = true;

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
  // Nothing to be done here. Suppress ZGlass implementation.
  // Sub-classes usually call ConsiderRebuildDL.
}

void ZRnrModBase_GL_Rnr::Draw(RnrDriver* rd)
{
  // Nothing to be done here. Suppress ZGlass implementation.
}

void ZRnrModBase_GL_Rnr::PostDraw(RnrDriver* rd)
{
  // Nothing to be done here. Suppress ZGlass implementation.
}

/**************************************************************************/

void ZRnrModBase_GL_Rnr::ConsiderRebuildDL(RnrDriver* rd)
{
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
