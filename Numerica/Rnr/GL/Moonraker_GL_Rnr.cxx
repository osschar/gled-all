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
  Moonraker      &M = *mMoonraker;
  const Double_t  T =  M.mT;

  glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT | GL_LIGHTING_BIT);
  glPushMatrix();
  glScalef(M.mScale, M.mScale, M.mScale);
  glColor4fv(M.mEColor());

  gluSphere(mQuadric, 1, M.mLOD, M.mLOD);

  // Moon
  glPushMatrix();
  Double_t mp[3]; M.MoonPos(mp, T);
  glTranslated(mp[0], mp[1], mp[2]);
  glColor4fv(M.mMColor());
  gluSphere(mQuadric, M.mRMoon, M.mLOD, M.mLOD);
  glPopMatrix();

  // Trajectory
  
  if (*M.mODECrawler)
  {
    ODEStorageD* S = dynamic_cast<ODEStorageD*>(M.mODECrawler->GetStorage());
    if (S && S->Size() > 0)
    {
      UInt_t N = S->Size();
      const Double_t *X = S->GetXArr();

      // Shell at mT
      if (T >= X[0] && T <= X[N-1])
      {
        // Bisect
        Int_t l = 0, h = N-1;
        while (h-l > 1)
        {
          Int_t m = TMath::Nint((T - X[l]) / (X[h] - X[l])*(h-l)) + l;
          if (m==l) m++;
          if (m==h) m--;
          if (X[m] <= T) l = m; else h = m;
        }
        Float_t f = (T - X[l]) / (X[h] - X[l]); // assume linear
        glColor4fv(M.mSColor());
        glPushMatrix();
        TVectorD yl; S->AssignY(l, yl);
        TVectorD yh; S->AssignY(h, yh);
        TVectorD p(yh); p -= yl; p *= f; p += yl;
        glTranslated(p(0), p(1), p(2));
        gluSphere(mQuadric, M.mRShell, M.mLOD, M.mLOD);
        glPopMatrix();
      }

      // Trajectory
      glDisable(GL_LIGHTING);
      glLineWidth(M.mTWidth);
      glColor4fv(M.mTColor());
      glBegin(GL_LINE_STRIP);
      for (UInt_t i=0; i<N; ++i)
      {
        //printf("\t%u\t%f\t%f\t%f\t%f\n", i, (*x)(i), (y[i])(0u), (y[i])(1u), (y[i])(2u));
        glVertex3dv(S->GetY(i));
      }
      glEnd();
    }
  }

  glPopMatrix();
  glPopAttrib();
}
