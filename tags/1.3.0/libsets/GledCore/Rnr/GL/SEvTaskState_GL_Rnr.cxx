// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SEvTaskState_GL_Rnr.h"
#include <Stones/ZColor.h>
#include <Rnr/GL/SphereTrings.h>

#include <GL/glew.h>

namespace {
  ZColor def_cols[4] = {
    ZColor( 0.1, 0.1, 0.8 ),
    ZColor( 0,   0.8, 0.2 ),
    ZColor( 0.8,   0, 0.2 ),
    ZColor( 0.1, 0.6, 0.6 )
  };
}

/**************************************************************************/

void SEvTaskState_GL_Rnr::RenderHisto(const SEvTaskState& ts, ZColor* cols)
{
  if(cols == 0) cols = def_cols;

  Float_t ak = 1.0/SphereTrings::CubeA;
  Float_t ah = SphereTrings::CubeA/2;

  Float_t nall = (ts.mNAll != 0) ? ts.mNAll : 1;

  glPushMatrix();
  glTranslatef(0.5, 0.5, 0);

  glColor4fv(cols[0]());
  glPushMatrix();
  glScalef(ak, ak, ak);
  glTranslatef(0, 0, ah);
  SphereTrings::Render(1, true);
  glPopMatrix();

  glTranslatef(1, 0, 0);

  glColor4fv(cols[1]());
  glPushMatrix();
  glScalef(ak, ak, ak * ts.mNOK / nall);
  glTranslatef(0, 0, ah);
  SphereTrings::Render(1, true);
  glPopMatrix();

  glTranslatef(1, 0, 0);

  glColor4fv(cols[2]());
  glPushMatrix();
  glScalef(ak, ak, ak * ts.mNFail / nall);
  glTranslatef(0, 0, ah);
  SphereTrings::Render(1, true);
  glPopMatrix();

  glTranslatef(1, 0, 0);

  glColor4fv(cols[3]());
  glPushMatrix();
  glScalef(ak, ak, ak * ts.mNProc / nall);
  glTranslatef(0, 0, ah);
  SphereTrings::Render(1, true);
  glPopMatrix();

  glPopMatrix();
}

void SEvTaskState_GL_Rnr::RenderBar(const SEvTaskState& ts, ZColor* cols)
{
  if(cols == 0) cols = def_cols;

  Float_t ak = 1.0/SphereTrings::CubeA;
  Float_t ah = SphereTrings::CubeA/2;

  Float_t nall = (ts.mNAll != 0) ? ts.mNAll : 1;
  Float_t yc, y = 0;


  // If NAll==0, render outline & RETURN.
  if(ts.mNAll == 0) {
    glColor4fv(cols[0]());
    glBegin(GL_LINE_LOOP);
    glVertex2i(0,0); glVertex2i(1,0); glVertex2i(1,1); glVertex2i(0,1);
    glEnd();
    return;
  }

  glPushMatrix();
  glTranslatef(0.5, 0.5, 0);

  // Failed
  if(ts.mNFail > 0) {
    yc = ts.mNFail / nall;
    glColor4fv(cols[2]());
    glPushMatrix();
    glScalef(ak, ak, ak * yc);
    glTranslatef(0, 0, ah);
    SphereTrings::Render(1, true);
    glPopMatrix();
    glTranslatef(0, 0, yc); y += yc;
  }

  // OK
  if(ts.mNOK > 0) {
    yc = ts.mNOK / nall;
    glColor4fv(cols[1]());
    glPushMatrix();
    glScalef(ak, ak, ak * yc);
    glTranslatef(0, 0, ah);
    SphereTrings::Render(1, true);
    glPopMatrix();
    glTranslatef(0, 0, yc); y += yc;
  }

  // Processing
  if(ts.mNProc > 0) {
    yc = ts.mNProc / nall;
    glColor4fv(cols[3]());
    glPushMatrix();
    glScalef(ak, ak, ak * yc);
    glTranslatef(0, 0, ah);
    SphereTrings::Render(1, true);
    glPopMatrix();
    glTranslatef(0, 0, yc); y += yc;
  }

  // The remaining
  if(ts.GetNLeft() > 0) {
    yc = 1 - y;
    glColor4fv(cols[0]());
    glPushMatrix();
    glScalef(ak, ak, ak * yc);
    glTranslatef(0, 0, ah);
    SphereTrings::Render(1, true);
    glPopMatrix();
  }

  glPopMatrix();
}
