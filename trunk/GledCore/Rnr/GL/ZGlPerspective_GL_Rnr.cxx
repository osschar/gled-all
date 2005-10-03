// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlPerspective_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>

#include <GL/gl.h>

#define PARENT ZRnrModBase_GL_Rnr

/**************************************************************************/

void ZGlPerspective_GL_Rnr::_init()
{}

/**************************************************************************/

void ZGlPerspective_GL_Rnr::setup_matrices(RnrDriver* rd, bool push_p)
{
  ZGlPerspective& M = *mZGlPerspective;

  if(M.mViewMode != ZGlPerspective::VM_Nop) {
    if(push_p) glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    if(push_p) glPushMatrix();
    glLoadMatrixd(rd->GetProjBase()->Array());
    switch(M.mViewMode) {
    case ZGlPerspective::VM_OrthoPixel:
      glOrtho(0, rd->GetWidth(), 0, rd->GetHeight(), M.mOrthoNear, M.mOrthoFar);
      break;
    case ZGlPerspective::VM_OrthoFixed:
      glOrtho(0, M.mOrthoW, 0, M.mOrthoH, M.mOrthoNear, M.mOrthoFar);
      break;
    default:
      break;
    }
    glMatrixMode(GL_MODELVIEW);
  }

  if(M.mViewMode == ZGlPerspective::VM_OrthoPixel) {
    Float_t x = M.mOx; if(x < 0) x += rd->GetWidth();
    Float_t y = M.mOy; if(y < 0) y += rd->GetHeight();
    glTranslatef(x, y, M.mOz);
  } else {
    glTranslatef(M.mOx, M.mOy, M.mOz);
  }
}

void ZGlPerspective_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);
  setup_matrices(rd, true);
}

void ZGlPerspective_GL_Rnr::Draw(RnrDriver* rd)
{
  setup_matrices(rd, false);
}

void ZGlPerspective_GL_Rnr::PostDraw(RnrDriver* rd)
{
  ZGlPerspective& M = *mZGlPerspective;
  if(M.mViewMode != ZGlPerspective::VM_Nop) {
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
  }
  PARENT::PostDraw(rd);
}
