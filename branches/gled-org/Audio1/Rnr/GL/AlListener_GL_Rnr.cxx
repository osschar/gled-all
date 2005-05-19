// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AlListener_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>

#include <AL/al.h>
#include <FL/gl.h>

/**************************************************************************/

void AlListener_GL_Rnr::_init()
{}

/**************************************************************************/

//void AlListener_GL_Rnr::PreDraw(RnrDriver* rd)
//{}

void AlListener_GL_Rnr::Draw(RnrDriver* rd)
{
  ZTrans& t = *rd->GetAbsCamera();
  alListener3f(AL_POSITION, t(1,4), t(2,4), t(3,4));
  printf("Listener: %f %f %f\n", t(1,4), t(2,4), t(3,4));
}

//void AlListener_GL_Rnr::PostDraw(RnrDriver* rd)
//{}
