// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Planetes_GL_Rnr.h"
#include "Rnr/GL/TringTvor_GL_Rnr.h"
#include "Rnr/GL/GLRnrDriver.h"

#include "Glasses/HTriMesh.h"

#include <GL/glew.h>

#define PARENT ZNode_GL_Rnr

//==============================================================================

void Planetes_GL_Rnr::_init()
{}

Planetes_GL_Rnr::Planetes_GL_Rnr(Planetes* idol) :
  ZNode_GL_Rnr(idol),
  mPlanetes(idol)
{
  _init();
}

Planetes_GL_Rnr::~Planetes_GL_Rnr()
{}

//==============================================================================

// void Planetes_GL_Rnr::PreDraw(RnrDriver* rd) {}

void Planetes_GL_Rnr::Draw(RnrDriver* rd)
{
  GET_OR_RET(HTriMesh, mesh, mPlanetes->GetMesh());

  rd->GL()->Color(mPlanetes->mColor);

  PARENT::Draw(rd);
}

// void Planetes_GL_Rnr::PostDraw(RnrDriver* rd) {}

void Planetes_GL_Rnr::Render(RnrDriver* rd)
{
  printf("Planetes_GL_Rnr::Render requested at level=%d\n",
	 mPlanetes->GetDrawLevel());

  GET_OR_RET(HTriMesh, mesh, mPlanetes->GetMesh());

  TringTvor_GL_Rnr::Render(mesh->GetTTvor());
}
