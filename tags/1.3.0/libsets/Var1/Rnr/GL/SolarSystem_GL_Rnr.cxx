// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SolarSystem_GL_Rnr.h"
#include <Glasses/CosmicBall.h>
#include <Rnr/GL/GLRnrDriver.h>
#include <TMath.h>

/**************************************************************************/

void SolarSystem_GL_Rnr::_init()
{}

/**************************************************************************/

//void SolarSystem_GL_Rnr::PreDraw(RnrDriver* rd){}

void SolarSystem_GL_Rnr::Draw(RnrDriver* rd)
{
  SolarSystem& SS = *mSolarSystem;

  const Double_t two_pi = TMath::TwoPi(), phi_step = two_pi/60.0;

  glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT);
  rd->GL()->Color(SS.RefColor());

  Double_t max_r = SS.mOrbitMaxR;

  glLineWidth(2);
  for (Double_t r = 100; r <= max_r; r += 100)
  {
    glBegin(GL_LINE_LOOP);
    for (Double_t phi = 0; phi < two_pi; phi += phi_step)
    {
      glVertex3d(r*TMath::Cos(phi), r*TMath::Sin(phi), 0);
    }
    glEnd();
  }
  glLineWidth(1);
  glLineStipple(4, 0x5555);
  glEnable(GL_LINE_STIPPLE);
  for (Double_t r = 20; r <= max_r; r += 20)
  {
    glBegin(GL_LINE_LOOP);
    for (Double_t phi = 0; phi < two_pi; phi += phi_step)
    {
      glVertex3d(r*TMath::Cos(phi), r*TMath::Sin(phi), 0);
    }
    glEnd();
  }
  glDisable(GL_LINE_STIPPLE);

  glLineWidth(2);
  rd->GL()->Color(1, 0, 0);
  glBegin(GL_LINES);
  TVector3 start;
  AList::Stepper<CosmicBall> stepper(SS.GetBalls());
  while (stepper.step())
  {
    const Double_t* P = stepper->RefTrans().ArrT();
    const TVector3& V = stepper->RefV();
    glVertex3d(P[0], P[1], P[2]);
    glVertex3d(P[0] + 100*V.x(), P[1] + 100*V.y(), P[2] + 100*V.z());
  }
  glEnd();

  glPopAttrib();
}

//void SolarSystem_GL_Rnr::PostDraw(RnrDriver* rd){}
