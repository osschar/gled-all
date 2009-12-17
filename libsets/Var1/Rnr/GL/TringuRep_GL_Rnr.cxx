// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuRep_GL_Rnr.h"
#include <GL/glew.h>

//==============================================================================

void TringuRep_GL_Rnr::_init()
{}

TringuRep_GL_Rnr::TringuRep_GL_Rnr(TringuRep* idol) :
  ZNode_GL_Rnr(idol),
  mTringuRep(idol)
{
  _init();
}

TringuRep_GL_Rnr::~TringuRep_GL_Rnr()
{}

//==============================================================================

void TringuRep_GL_Rnr::PreDraw(RnrDriver* rd) {}

void TringuRep_GL_Rnr::Draw(RnrDriver* rd) {}

void TringuRep_GL_Rnr::PostDraw(RnrDriver* rd) {}

void TringuRep_GL_Rnr::Render(RnrDriver* rd) {}
