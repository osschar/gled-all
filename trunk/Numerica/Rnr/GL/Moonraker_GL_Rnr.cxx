// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Moonraker_GL_Rnr.h"

#include <TMath.h>

void Moonraker_GL_Rnr::_init()
{
  mQuadric = gluNewQuadric();
  gluQuadricDrawStyle(mQuadric, GLU_FILL);
  gluQuadricNormals(mQuadric, GLU_SMOOTH);
}

Moonraker_GL_Rnr::~Moonraker_GL_Rnr()
{
  gluDeleteQuadric(mQuadric);
}

/**************************************************************************/

void Moonraker_GL_Rnr::Draw(RnrDriver* rd)
{
  glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_LIGHTING_BIT);
  glPushMatrix();
  glScalef(mMoonraker->mScale, mMoonraker->mScale, mMoonraker->mScale);
  glColor4fv(mMoonraker->mEColor());

  gluSphere(mQuadric, 1, mMoonraker->mLOD, mMoonraker->mLOD);

  // Moon
  glPushMatrix();
  Double_t mp[3]; mMoonraker->MoonPos(mp, mMoonraker->mT);
  glTranslated(mp[0], mp[1], mp[2]);
  glColor4fv(mMoonraker->mMColor());
  gluSphere(mQuadric, mMoonraker->mRMoon, mMoonraker->mLOD, mMoonraker->mLOD);
  glPopMatrix();

  // Trajectory
  if(*mMoonraker->mODECrawler && mMoonraker->mODECrawler->GetStored()) {
    UInt_t M = mMoonraker->mODECrawler->GetStored();
    TVectorF *x = mMoonraker->mODECrawler->GetXStored();
    TVectorF *y = mMoonraker->mODECrawler->GetYStored();

    // Shell at mT
    if(mMoonraker->mT>=(*x)(0u) && mMoonraker->mT<=(*x)(M-1)) {
      // Bisect
      UInt_t l=0, h=M-1;
      while(h-l > 1) {
	UInt_t m = TMath::Nint((mMoonraker->mT-(*x)(l))/((*x)(h)-(*x)(l))*(h-l)) + l;
	if(m==l) m++; if(m==h) m--;
	if((*x)(m) <= mMoonraker->mT) l = m; else h = m;
      }
      Float_t f = (mMoonraker->mT-(*x)(l))/((*x)(h)-(*x)(l)); // assume linear
      glColor4fv(mMoonraker->mSColor());
      glPushMatrix();
      TVectorF p(y[h]); p -= y[l]; p *= f; p += y[l];
      glTranslatef(p(0u), p(1u), p(2u));
      gluSphere(mQuadric, mMoonraker->mRShell, mMoonraker->mLOD, mMoonraker->mLOD);
      glPopMatrix();
    }

    // Trajectory
    glDisable(GL_LIGHTING);
    glLineWidth(mMoonraker->mTWidth);
    glColor4fv(mMoonraker->mTColor());
    glBegin(GL_LINE_STRIP);
    for(UInt_t i=0; i<M; i++) {
      //printf("\t%u\t%f\t%f\t%f\t%f\n", i, (*x)(i), (y[i])(0u), (y[i])(1u), (y[i])(2u));
      glVertex3fv(y[i].GetMatrixArray());
    }
    glEnd();
  }

  glPopMatrix();

  glPopAttrib();
}
