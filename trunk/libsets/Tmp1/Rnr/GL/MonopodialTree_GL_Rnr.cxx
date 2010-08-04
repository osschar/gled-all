// $Id: Rnr_GL_SKEL.cxx 2220 2009-07-09 21:20:00Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "MonopodialTree_GL_Rnr.h"
#include <GL/glew.h>

//==============================================================================

void MonopodialTree_GL_Rnr::_init()
{}

MonopodialTree_GL_Rnr::MonopodialTree_GL_Rnr(MonopodialTree* idol) :
  ParametricSystem_GL_Rnr(idol),
  mMonopodialTree(idol)
{
  _init();
}

MonopodialTree_GL_Rnr::~MonopodialTree_GL_Rnr()
{}

//==============================================================================
/*
void MonopodialTree_GL_Rnr::PreDraw(RnrDriver* rd) {}

void MonopodialTree_GL_Rnr::Draw(RnrDriver* rd) {}

void MonopodialTree_GL_Rnr::PostDraw(RnrDriver* rd) {}
*/

void MonopodialTree_GL_Rnr::Triangulate(RnrDriver* rd) 
{
  mMonopodialTree->Produce();
}
