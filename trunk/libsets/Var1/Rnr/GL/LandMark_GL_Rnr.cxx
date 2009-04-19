// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "LandMark_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/TringTvor_GL_Rnr.h>

#include <GL/glew.h>

#define PARENT Extendio_GL_Rnr

/**************************************************************************/

void LandMark_GL_Rnr::_init()
{}

/**************************************************************************/

//void LandMark_GL_Rnr::PreDraw(RnrDriver* rd){}

//void LandMark_GL_Rnr::Draw(RnrDriver* rd){}

void LandMark_GL_Rnr::Render(RnrDriver* rd)
{
  LandMark &LM = * mLandMark;

  glPushAttrib(GL_ENABLE_BIT);

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  rd->GL()->Color(LM.mColor);

  TringTvor_GL_Rnr::Render(LM.GetMesh()->GetTTvor(), false);

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  glLineWidth(2);
  glBegin(GL_LINE_LOOP);
  glVertex3f(LM.mSx, 0, 0);
  Float_t d = TMath::TwoPi()/32;
  Float_t s = 0;
  for (Int_t i = 1; i < 32; ++i)
  {
    s += d;
    glVertex3f(LM.mSx*cosf(s), LM.mSy*sinf(s), 0);
  }
  glEnd();

  glPopAttrib();
}

//void LandMark_GL_Rnr::PostDraw(RnrDriver* rd){}
