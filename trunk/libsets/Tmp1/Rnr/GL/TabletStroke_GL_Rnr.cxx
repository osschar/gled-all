// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletStroke_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>

#include <GL/glew.h>

//==============================================================================

void TabletStroke_GL_Rnr::_init()
{}

TabletStroke_GL_Rnr::TabletStroke_GL_Rnr(TabletStroke* idol) :
  ZNode_GL_Rnr(idol),
  mTabletStroke(idol)
{
  _init();
}

TabletStroke_GL_Rnr::~TabletStroke_GL_Rnr()
{}

//==============================================================================

//void TabletStroke_GL_Rnr::PreDraw(RnrDriver* rd) {}

//void TabletStroke_GL_Rnr::Draw(RnrDriver* rd) {}

//void TabletStroke_GL_Rnr::PostDraw(RnrDriver* rd) {}

void TabletStroke_GL_Rnr::Render(RnrDriver* rd)
{
  vector<HPointF>& P = mTabletStroke->mPoints;

  rd->GL()->Color(mTabletStroke->mColor);

  glBegin(GL_POINTS);
  for (Int_t i = 0; i < P.size(); ++i)
    glVertex3fv(P[i]);
  glEnd();
}
