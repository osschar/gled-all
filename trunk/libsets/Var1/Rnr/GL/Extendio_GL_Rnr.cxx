// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Extendio_GL_Rnr.h"

#include <Glasses/Tringula.h>

#include <Rnr/GL/TringTvor_GL_Rnr.h>

#include <GL/glew.h>

#define PARENT ZGlass_GL_Rnr

/**************************************************************************/

void Extendio_GL_Rnr::_init()
{}

/**************************************************************************/

void Extendio_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);

  glPushMatrix();

  glMultMatrixf(mExtendio->RefLastTrans().Array());
}

void Extendio_GL_Rnr::Draw(RnrDriver* rd)
{
  GET_OR_RET(TriMesh, mesh, mExtendio->GetMesh());

  PARENT::Draw(rd);

  Extendio &E = * mExtendio;
  Tringula &T = * mExtendio->mTringula;

  if (T.GetRnrBBoxes())
  {
    GL_Capability_Switch ligt_off(GL_LIGHTING, false);
    glColor3f(1, 0, 0);
    TringTvor_GL_Rnr::RenderCEBBox(mesh->GetTTvor()->mCtrExtBox, 1.01f);
  }
}

void Extendio_GL_Rnr::Render(RnrDriver* rd)
{
  GET_OR_RET(TriMesh, mesh, mExtendio->GetMesh());

  TringTvor_GL_Rnr::Render(mesh->GetTTvor(), false);
}

void Extendio_GL_Rnr::PostDraw(RnrDriver* rd)
{
  glPopMatrix();

  Extendio &E = * mExtendio;
  Tringula &T = * mExtendio->mTringula;

  if (T.GetRnrBBoxes())
  {
    GL_Capability_Switch ligt_off(GL_LIGHTING, false);
    glColor3f(0, 0, 1);
    TringTvor_GL_Rnr::RenderCEBBox((Float_t*)&E.RefLastAABB(), 1.01f);
  }

  PARENT::PostDraw(rd);
}
