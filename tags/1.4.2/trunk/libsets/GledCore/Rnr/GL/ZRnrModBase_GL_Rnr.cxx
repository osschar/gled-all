// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRnrModBase_GL_Rnr.h"
#include <GL/glew.h>

#define PARENT ZGlass_GL_Rnr

/**************************************************************************/

void ZRnrModBase_GL_Rnr::_init()
{
  mRnrMod = new RnrMod(mZRnrModBase, this);
}

ZRnrModBase_GL_Rnr::~ZRnrModBase_GL_Rnr()
{
  delete mRnrMod;
}

/**************************************************************************/

void ZRnrModBase_GL_Rnr::update_tring_stamp(RnrDriver* rd)
{
  // Usually called by sub-classes from PreDraw and Draw.

  mRnrMod->fTringTS = mZRnrModBase->GetStampReqTring();
}
