// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "WSSeed_GL_Rnr.h"
#include <Rnr/GL/TubeTvor_GL_Rnr.h>
#include <Glasses/WSPoint.h>

#include <GL/gl.h>

/**************************************************************************/

void WSSeed_GL_Rnr::vert(WSPoint* f, Real_t t)
{
  Real_t x[3];
  Real_t t2 = t*t;
  Real_t t3 = t2*t;
  for(UInt_t i=1; i<=3; i++)
    x[i-1] = f->mCoffs(i,0u) + f->mCoffs(i,1u)*t +
             f->mCoffs(i,2u)*t2 + f->mCoffs(i,3u)*t3;
  glVertex3fv(x);
}

/**************************************************************************/

void WSSeed_GL_Rnr::Draw(RnrDriver* rd)
{
  // Self rendering missing. But ... self parameters also missing.

  glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT);

  glLineWidth(mWSSeed->mLineW);
  glColor4fv(mWSSeed->mColor());
  if(mWSSeed->bFat) {
    if(mWSSeed->pTuber) TubeTvor_GL_Rnr::Render(mWSSeed->pTuber);
  } else {
    list<WSPoint*> points; mWSSeed->CopyByGlass<WSPoint*>(points);
    int size = points.size();
    if(size >= 2) {
      list<WSPoint*>::iterator a = points.begin();
      list<WSPoint*>::iterator b;
      while((b = a, ++b) != points.end()) {
	(*a)->Coff(*b);
	Real_t delta = 1.0/mWSSeed->mTLevel, max = 1 - delta/2;
	glBegin(GL_LINE_STRIP);
	for(Real_t t=0; t<max; t+=delta) {
	  vert(*a, t);
	}
	vert(*a, 1);
	glEnd();
	a = b;
      }
    }
  }

  glPopAttrib();
}

void WSSeed_GL_Rnr::Triangulate()
{
  ZNode_GL_Rnr::Triangulate();
  mWSSeed->Triangulate();
}
