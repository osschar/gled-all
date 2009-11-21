// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TSPupilInfo_GL_Rnr.h"
#include <GL/glew.h>

//==============================================================================

void TSPupilInfo_GL_Rnr::_init()
{}

TSPupilInfo_GL_Rnr::TSPupilInfo_GL_Rnr(TSPupilInfo* idol) :
  PupilInfo_GL_Rnr(idol),
  mTSPupilInfo(idol)
{
  _init();
}

TSPupilInfo_GL_Rnr::~TSPupilInfo_GL_Rnr()
{}

//==============================================================================

// void TSPupilInfo_GL_Rnr::PreDraw(RnrDriver* rd) {}
// void TSPupilInfo_GL_Rnr::Draw(RnrDriver* rd) {}
// void TSPupilInfo_GL_Rnr::PostDraw(RnrDriver* rd) {}
// void TSPupilInfo_GL_Rnr::Render(RnrDriver* rd) {}
