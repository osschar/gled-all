// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ExtendioExplosion_GL_Rnr.h"
#include <Glasses/Extendio.h>

#include <Rnr/GL/GLRnrDriver.h>
#include <GL/glew.h>

#define PARENT Explosion_GL_Rnr

//==============================================================================

ExtendioExplosion_GL_Rnr::ExtendioExplosion_GL_Rnr(ExtendioExplosion* idol) :
  Explosion_GL_Rnr(idol),
  mExtendioExplosion(idol)
{}

ExtendioExplosion_GL_Rnr::~ExtendioExplosion_GL_Rnr()
{}

//==============================================================================

void ExtendioExplosion_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);
  glPushMatrix();
  glMultMatrixf(mExtendioExplosion->GetExtendio()->RefLastTrans().Array());
}

void ExtendioExplosion_GL_Rnr::Draw(RnrDriver* rd)
{
  ExtendioExplosion &EE = * mExtendioExplosion;

  const Float_t nt = EE.mExplodeTime / EE.mExplodeDuration;
  const Float_t bf = 0.6f + 0.7f*nt;
  const Float_t sf = 1.0f + 1.2f*nt;

  Float_t *bb = mExtendioExplosion->GetExtendio()->get_tring_tvor()->mCtrExtBox;

  glTranslatef(bb[0], bb[1], 0.0f);

  glScalef(bf*bb[3], bf*bb[4], 2.0f*bf*bb[5]);
  glColor4f(1.0f - 0.5f*nt, 1.0f - nt, 0.0f, 1.0f - nt);
  PARENT::Draw(rd);

  glScalef(sf, sf, sf);
  glColor4f(1.0f - nt, 0.0f, 0.0f, 0.5f - 0.5f*nt);
  PARENT::Draw(rd);
}

void ExtendioExplosion_GL_Rnr::PostDraw(RnrDriver* rd)
{
  glPopMatrix();
}

void ExtendioExplosion_GL_Rnr::Render(RnrDriver* rd)
{
  gluSphere(rd->GL()->GetQuadricStdNoNormals(), 1.0, 30, 30);
}
