// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Explosion_GL_Rnr.h"
#include <GL/glew.h>

//==============================================================================

void Explosion_GL_Rnr::_init()
{}

Explosion_GL_Rnr::Explosion_GL_Rnr(Explosion* idol) :
  ZGlass_GL_Rnr(idol),
  mExplosion(idol)
{
  _init();
}

Explosion_GL_Rnr::~Explosion_GL_Rnr()
{}

//==============================================================================

// void Explosion_GL_Rnr::PreDraw(RnrDriver* rd) {}

// void Explosion_GL_Rnr::Draw(RnrDriver* rd) {}

// void Explosion_GL_Rnr::PostDraw(RnrDriver* rd) {}

// void Explosion_GL_Rnr::Render(RnrDriver* rd) {}
