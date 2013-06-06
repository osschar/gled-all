// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletReader_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>

#include <GL/glew.h>

#include <cmath>

#define PARENT ZNode_GL_Rnr

//==============================================================================

void TabletReader_GL_Rnr::_init()
{}

TabletReader_GL_Rnr::TabletReader_GL_Rnr(TabletReader* idol) :
  ZNode_GL_Rnr(idol),
  mTabletReader(idol),
  mTabletRMS()
{
  _init();
}

TabletReader_GL_Rnr::~TabletReader_GL_Rnr()
{}

//==============================================================================

//void TabletReader_GL_Rnr::PreDraw(RnrDriver* rd) {}

void TabletReader_GL_Rnr::Draw(RnrDriver* rd)
{
  obtain_rnrmod(rd, mTabletRMS);
  PARENT::Draw(rd);
}

//void TabletReader_GL_Rnr::PostDraw(RnrDriver* rd) {}

void TabletReader_GL_Rnr::Render(RnrDriver* rd)
{
  TabletRnrMod &RM = * mTabletRMS.lens();
  TabletReader &R  = * mTabletReader;

  Float_t d = RM.GetMarkSize();
  if (d <= 0)
    return;

  glPushMatrix();
  glTranslatef(R.mPenX, R.mPenY, 0.001f);

  if (R.bInTouch)
  {
    d *= powf(R.mPenP, RM.GetPressCurveAlpha());
    rd->GL()->Color(RM.RefInTouchColor());
    glBegin(GL_LINE_LOOP);
    Float_t astep = TMath::TwoPi()/20;
    Float_t phi = 0;
    for (Int_t i = 0; i < 20; ++i, phi+=astep)
    {
      glVertex2f(d*cosf(phi), d*sinf(phi));
    }
    glEnd();
  }
  else if (R.bInProximity)
  {
    Float_t e = 0.5f * d;
    rd->GL()->Color(RM.RefInProximityColor());
    glBegin(GL_LINES);
    glVertex2f( d,  0); glVertex2f( e,  0);
    glVertex2f(-e,  0); glVertex2f(-d,  0);
    glVertex2f( 0,  d); glVertex2f( 0,  e);
    glVertex2f( 0, -e); glVertex2f( 0, -d);
    glEnd();
  }

  glPopMatrix();
}
