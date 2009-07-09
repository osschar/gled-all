// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CLASSNAME_GL_Rnr.h"
#include <GL/glew.h>

//==============================================================================

void CLASSNAME_GL_Rnr::_init()
{}

CLASSNAME_GL_Rnr::CLASSNAME_GL_Rnr(CLASSNAME* idol) :
  BASENAME_GL_Rnr(idol),
  mCLASSNAME(idol)
{
  _init();
}

CLASSNAME_GL_Rnr::~CLASSNAME_GL_Rnr()
{}

//==============================================================================

void CLASSNAME_GL_Rnr::PreDraw(RnrDriver* rd) {}

void CLASSNAME_GL_Rnr::Draw(RnrDriver* rd) {}

void CLASSNAME_GL_Rnr::PostDraw(RnrDriver* rd) {}

void CLASSNAME_GL_Rnr::Render(RnrDriver* rd) {}
