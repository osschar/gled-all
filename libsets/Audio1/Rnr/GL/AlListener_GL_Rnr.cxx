// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AlListener_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>

#include <AL/alut.h>
#include <GL/glew.h>

/**************************************************************************/

void AlListener_GL_Rnr::_init()
{}

/**************************************************************************/

//void AlListener_GL_Rnr::PreDraw(RnrDriver* rd)
//{}

void AlListener_GL_Rnr::Draw(RnrDriver* rd)
{
  ZTrans* tp = 0;
  switch (mAlListener->mLocationType)
  {
    case AlListener::LT_Camera:   tp = rd->GetCamAbsTrans(); break;
    case AlListener::LT_Absolute: tp = &rd->ToGCS();         break;
  }
  ZTrans&  t = *tp;
  Float_t orient[6] = { (float) t(1,1), (float) t(2,1), (float) t(3,1),
                        (float) t(1,3), (float) t(2,3), (float) t(3,3) };
  alListener3f(AL_POSITION,    t(1,4), t(2,4), t(3,4));
  alListenerfv(AL_ORIENTATION, orient);
}

//void AlListener_GL_Rnr::PostDraw(RnrDriver* rd)
//{}
