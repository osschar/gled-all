// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ScreenText_GL_Rnr.h"
#include <GledView/GLTextNS.h>
#include <FL/gl.h>

/**************************************************************************/

void ScreenText_GL_Rnr::_init()
{}

/**************************************************************************/

//void ScreenText_GL_Rnr::PreDraw(RnrDriver* rd)
//{}

void ScreenText_GL_Rnr::Draw(RnrDriver* rd)
{
  ScreenText& T = *mScreenText;
  GLTextNS::RnrTextAt(rd, T.GetText(), T.mX, T.mY, T.mZ, T.PtrFgCol(), T.PtrBgCol());
}

//void ScreenText_GL_Rnr::PostDraw(RnrDriver* rd)
//{}
