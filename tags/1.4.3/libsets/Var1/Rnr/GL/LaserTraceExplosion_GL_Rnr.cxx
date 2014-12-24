// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "LaserTraceExplosion_GL_Rnr.h"

#include <Rnr/GL/GLRnrDriver.h>
#include <GL/glew.h>

#define PARENT Explosion_GL_Rnr

//==============================================================================

void LaserTraceExplosion_GL_Rnr::_init()
{}

LaserTraceExplosion_GL_Rnr::LaserTraceExplosion_GL_Rnr(LaserTraceExplosion* idol) :
  Explosion_GL_Rnr(idol),
  mLaserTraceExplosion(idol)
{
  _init();
}

LaserTraceExplosion_GL_Rnr::~LaserTraceExplosion_GL_Rnr()
{}

//==============================================================================

//void LaserTraceExplosion_GL_Rnr::PreDraw(RnrDriver* rd) {}

void LaserTraceExplosion_GL_Rnr::Draw(RnrDriver* rd)
{
  LaserTraceExplosion &LTE = * mLaserTraceExplosion;

  const Float_t nt = LTE.mExplodeTime / LTE.mExplodeDuration;
  const Float_t x  = 1.0f - nt;
  glColor4f(x, x, x, x);
  GL_Float_Holder(GL_LINE_WIDTH, 3.0f, glLineWidth);

  PARENT::Draw(rd);
}

//void LaserTraceExplosion_GL_Rnr::PostDraw(RnrDriver* rd) {}

void LaserTraceExplosion_GL_Rnr::Render(RnrDriver* rd)
{
  LaserTraceExplosion &LTE = * mLaserTraceExplosion;

  if (LTE.mEndRadius > 0)
  {
    glPushMatrix();
    glTranslatef(LTE.mB.x, LTE.mB.y, LTE.mB.z);
    gluSphere(rd->GL()->GetQuadricStdNoNormals(), LTE.mEndRadius, 30, 30);
    glPopMatrix();
  }

  glBegin(GL_LINES);
  glVertex3fv(LTE.mA);
  glVertex3fv(LTE.mB);
  glEnd();
}
