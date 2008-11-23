// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZRnrModList_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <GL/gl.h>

namespace OS = OptoStructs;

#define PARENT ZGlass_GL_Rnr

/**************************************************************************/

void ZRnrModList_GL_Rnr::_init()
{}

/**************************************************************************/

// Could further optimize with two rnr-schemes for pre/post draw.
// Also need AbsorbRay to destroy them on list-change.

void ZRnrModList_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);
  OS::lpZGlassImg_t* imgs = fImg->GetElementImgs();
  for(OS::lpZGlassImg_i img=imgs->begin(); img!=imgs->end(); ++img)
    rd->GetRnr(*img)->PreDraw(rd);
}

void ZRnrModList_GL_Rnr::PostDraw(RnrDriver* rd)
{
  OS::lpZGlassImg_t* imgs = fImg->GetElementImgs();
  for(OS::lpZGlassImg_ri img=imgs->rbegin(); img!=imgs->rend(); ++img)
    rd->GetRnr(*img)->PostDraw(rd);
  PARENT::PostDraw(rd);
}
