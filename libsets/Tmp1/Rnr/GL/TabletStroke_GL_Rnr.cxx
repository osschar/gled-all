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
  TabletStroke    &S = * mTabletStroke;
  vSTabletPoint_t &P = S.mPoints;

  Int_t m, M;
  S.get_draw_range(m, M);

  if (M < 0)
    return;

  rd->GL()->Color(S.mPointColor);
  glBegin(GL_POINTS);
  for (Int_t i = m; i <= M; ++i)
  {
    if (P[i].p != 0)
      glVertex3fv(P[i]);
  }
  glEnd();

  if (M > m && ! S.bInStroke)
  {
    glColor4f(1, 0, 0, 1);
    glBegin(GL_POINTS);
    glVertex3fv(P[m-1]);
    glVertex3fv(P[M+1]);
    glEnd();
  }

  rd->GL()->Color(0.25f*S.mPointColor);
  glBegin(GL_POINTS);
  for (Int_t i = m; i <= M; ++i)
  {
    if (P[i].p == 0)
      glVertex3fv(P[i]);
  }
  glEnd();
}
