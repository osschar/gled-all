// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AlSource_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <FL/gl.h>

/**************************************************************************/

void AlSource_GL_Rnr::_init()
{}

/**************************************************************************/

//void AlSource_GL_Rnr::PreDraw(RnrDriver* rd)
//{}

void AlSource_GL_Rnr::Draw(RnrDriver* rd)
{
  if(mAlSource->mAlSrc) {
    ZTrans& t = rd->ToGCS();
    alSource3f(mAlSource->mAlSrc, AL_POSITION, t(1,4), t(2,4), t(3,4));
    // printf("Source: %f %f %f\n", t(1,4), t(2,4), t(3,4));
    alSource3f(mAlSource->mAlSrc, AL_DIRECTION, t(1,1), t(2,1), t(3,1));
  }
}

//void AlSource_GL_Rnr::PostDraw(RnrDriver* rd)
//{}
