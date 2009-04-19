// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Extendio_GL_Rnr.h"

#include <Glasses/Tringula.h>

#include <Rnr/GL/TringTvor_GL_Rnr.h>

#include <Rnr/GL/SphereTrings.h>

#include <GL/glew.h>

#define PARENT ZGlass_GL_Rnr

/**************************************************************************/

void Extendio_GL_Rnr::_init()
{}

/**************************************************************************/

void Extendio_GL_Rnr::render_ceaabox(const Float_t* x, Float_t f)
{
  // Render center-extents axis-aligned bounding-box.
  // f is additional scale of the extents.

  f *= 2;
  glPushMatrix();
  glTranslatef(x[0]-x[3], x[1]-x[4], x[2]-x[5]);
  glScalef(f*x[3], f*x[4], f*x[5]);
  SphereTrings::UnitFrameBox();
  glPopMatrix();
}

/**************************************************************************/

void Extendio_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);

  glPushMatrix();

  glMultMatrixf(mExtendio->RefLastTrans().Array());
}

void Extendio_GL_Rnr::Draw(RnrDriver* rd)
{
  PARENT::Draw(rd);

  Extendio &E = * mExtendio;
  Tringula &T = * mExtendio->mTringula;

  if (E.GetSelected())
  {
    GL_Capability_Switch ligt_off(GL_LIGHTING, false);
    GL_Float_Holder      fat_line(GL_LINE_WIDTH, 2, glLineWidth);
    glColor3fv(T.PtrSelColor()->array());
    render_ceaabox(E.GetMesh()->GetTTvor()->mCtrExtBox, 1.01f);
  }

  if (T.GetRnrBBoxes())
  {
    GL_Capability_Switch ligt_off(GL_LIGHTING, false);
    glColor3f(1, 0, 0);
    render_ceaabox(E.GetMesh()->GetTTvor()->mCtrExtBox, 1.01f);
  }
}

void Extendio_GL_Rnr::Render(RnrDriver* rd)
{
  TringTvor_GL_Rnr::Render(mExtendio->GetMesh()->GetTTvor(), false);
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
    render_ceaabox((Float_t*)&E.RefLastAABB(), 1.01f);
  }

  PARENT::PostDraw(rd);
}
