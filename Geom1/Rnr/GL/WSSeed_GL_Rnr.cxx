// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "WSSeed_GL_Rnr.h"
#include <Rnr/GL/TubeTvor_GL_Rnr.h>
#include <Glasses/WSPoint.h>

#include <GL/gl.h>

/**************************************************************************/

void WSSeed_GL_Rnr::vert(WSPoint* f, Float_t t)
{
  Float_t x[3];
  Float_t t2 = t*t;
  Float_t t3 = t2*t;
  for(UInt_t i=1; i<=3; i++)
    x[i-1] = f->mCoffs(i,0u) + f->mCoffs(i,1u)*t +
             f->mCoffs(i,2u)*t2 + f->mCoffs(i,3u)*t3;
  glVertex3fv(x);
}

/**************************************************************************/

void WSSeed_GL_Rnr::PreDraw(RnrDriver* rd)
{
  ZNode_GL_Rnr::PreDraw(rd);
  if(mWSSeed->mStampTexDone < mWSSeed->mStampReqTex) {
    mWSSeed->ReTexturize();
  }
  if(mStampTex != mWSSeed->mStampTexDone) {
    bRebuildDL = true;    
    mStampTex = mWSSeed->mStampTexDone;
  }
}

/**************************************************************************/

void WSSeed_GL_Rnr::Render(RnrDriver* rd)
{
  // Self rendering missing. But ... self parameters also missing.

  glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT | GL_LIGHTING_BIT);

  glLineWidth(mWSSeed->mLineW);
  glColor4fv(mWSSeed->mColor());
  if(mWSSeed->bFat) {
    if(mWSSeed->pTuber) TubeTvor_GL_Rnr::Render(mWSSeed->pTuber);
  } else {
    list<WSPoint*> points; mWSSeed->CopyByGlass<WSPoint*>(points);
    if(points.size() >= 2) {
      list<WSPoint*>::iterator a = points.begin();
      list<WSPoint*>::iterator b;
      while((b = a, ++b) != points.end()) {
	(*a)->Coff(*b);
	Float_t delta = 1.0/mWSSeed->mTLevel, max = 1 - delta/2;
	glBegin(GL_LINE_STRIP);
	// Texturing is fooed!!!
	// Need proper length and stretch etc ...
	Float_t utex = mWSSeed->mTexUOffset;
	Float_t vtex = mWSSeed->mTexVOffset;
	for(Float_t t=0; t<max; t+=delta) {
	  glTexCoord2f(utex + t, vtex);
	  vert(*a, t);
	}
	glTexCoord2f(utex + 1, vtex);
	vert(*a, 1);
	glEnd();
	a = b;
      }
    }
  }

  glPopAttrib();
}

void WSSeed_GL_Rnr::Triangulate(RnrDriver* rd)
{
  ZNode_GL_Rnr::Triangulate(rd);
  mWSSeed->Triangulate();
}
