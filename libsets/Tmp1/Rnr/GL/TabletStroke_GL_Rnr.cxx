// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletStroke_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>

// #include "TSpline.h"

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

  rd->GL()->Color(S.mColor);
  glBegin(GL_POINTS);
  for (Int_t i = m; i <= M; ++i)
  {
    if (P[i].p != 0)
      glVertex3fv(P[i]);
  }
  glEnd();

  rd->GL()->Color(0.25f*S.mColor);
  glBegin(GL_POINTS);
  for (Int_t i = m; i <= M; ++i)
  {
    if (P[i].p == 0)
      glVertex3fv(P[i]);
  }
  glEnd();

  // TSpline *sx = S.mSplineX;
  // TSpline *sy = S.mSplineY;
  // if (sx && sy)
  // {
  //   Float_t tm = S.MinT(), tM = S.MaxT();
  //   Float_t ts = (tM - tm) / 1000;
  //   Float_t t  = 0;
  //   rd->GL()->Color(S.mLineColor);
  //   glBegin(GL_LINE_STRIP);
  //   for (Int_t i = 0; i <= 1000; ++i, t+=ts)
  //   {
  //     glVertex2d(sx->Eval(t), sy->Eval(t));
  //   }
  //   glEnd();
  // }
}
