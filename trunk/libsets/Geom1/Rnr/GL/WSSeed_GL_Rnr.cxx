// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "WSSeed_GL_Rnr.h"
#include <Rnr/GL/TubeTvor_GL_Rnr.h>
#include <Rnr/GL/GLRnrDriver.h>
#include <Glasses/WSPoint.h>

#include <GL/gl.h>

#define PARENT ZNode_GL_Rnr

/**************************************************************************/

void WSSeed_GL_Rnr::vert(WSPoint* f, Float_t t)
{
  Double_t x[3], t2 = t*t, t3 = t2*t;
  for(UInt_t i=0; i<3; i++)
    x[i] = f->mCoffs(i,0) + f->mCoffs(i,1)*t +
      f->mCoffs(i,2)*t2 + f->mCoffs(i,3)*t3;
  glVertex3dv(x);
}

/**************************************************************************/

void WSSeed_GL_Rnr::Draw(RnrDriver* rd)
{
  glMatrixMode(GL_TEXTURE);
  glPushMatrix();
  glTranslatef(mWSSeed->mTexUOffset, mWSSeed->mTexVOffset, 0);
  glScalef(mWSSeed->mTexUScale, mWSSeed->mTexVScale, 1);
  glMatrixMode(GL_MODELVIEW);

  PARENT::Draw(rd);

  glMatrixMode(GL_TEXTURE);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

/**************************************************************************/

void WSSeed_GL_Rnr::Render(RnrDriver* rd)
{
  // Self rendering missing. But ... self parameters also missing.

  glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT);

  rd->GL()->LineWidth(mWSSeed->mLineW);
  rd->GL()->Color(mWSSeed->mColor);
  if(mWSSeed->bFat) {
    if(mWSSeed->pTuber) TubeTvor_GL_Rnr::Render(mWSSeed->pTuber);
  } else {
    GMutexHolder lstlck(mWSSeed->mListMutex);
    AList::Stepper<WSPoint> s(mWSSeed);
    s.step();
    WSPoint* a = *s;

    const Float_t delta = 1.0/mWSSeed->mTLevel, max = 1 - delta/2;
    Float_t utex = 0, vtex = 0;
    glBegin(GL_LINE_STRIP);
    while(s.step()) {
      a->Coff(*s);
      for(Float_t t=0; t<max; t+=delta) {
	glTexCoord2f(utex + t*a->mStretch, vtex + t*a->mTwist);
	vert(a, t);
      }
      utex += a->mStretch; vtex += a->mTwist;
      glTexCoord2f(utex, vtex);
      vert(a, 1);
      a = *s;
    }
    glEnd();
  }

  glPopAttrib();
}

void WSSeed_GL_Rnr::Triangulate(RnrDriver* rd)
{
  ZNode_GL_Rnr::Triangulate(rd);
  mWSSeed->Triangulate();
}
