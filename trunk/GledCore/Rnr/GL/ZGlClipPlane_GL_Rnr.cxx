// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlClipPlane_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/GLRnrDriver.h>
#include <GL/gl.h>

/**************************************************************************/

void ZGlClipPlane_GL_Rnr::_init()
{
  mClipId = -1;
}

/**************************************************************************/

void ZGlClipPlane_GL_Rnr::PreDraw(RnrDriver* rd)
{
  if(mZGlClipPlane->bRnrSelf)
    RnrSelf();
  TurnOn(rd);
}

void ZGlClipPlane_GL_Rnr::Draw(RnrDriver* rd)
{
  ZGlClipPlane& cp = *mZGlClipPlane;

  if(cp.bRnrSelf)
    RnrSelf();

  if(mClipId == -1) {
    if(cp.bOnIfOff)
      TurnOn(rd);
  } else {
    if(cp.bOffIfOn)
      TurnOff(rd);
  }
}

void ZGlClipPlane_GL_Rnr::PostDraw(RnrDriver* rd)
{
  TurnOff(rd);
}

/**************************************************************************/

void ZGlClipPlane_GL_Rnr::CleanUp(RnrDriver* rd)
{
  TurnOff(rd);
}

/**************************************************************************/

void ZGlClipPlane_GL_Rnr::TurnOn(RnrDriver* rd)
{
  if(mClipId >= 0) return;
  mClipId = rd->GL()->GetClipPlane(this);
  if(mClipId >= 0) {
    ZGlClipPlane& cp = *mZGlClipPlane;
    GLdouble plane[4];
    const Double_t
      theta = cp.fTheta * TMath::DegToRad(),
      phi   = cp.fPhi   * TMath::DegToRad(),
      cth   = TMath::Cos(theta);
    plane[0] = cth * TMath::Cos(phi);
    plane[1] = cth * TMath::Sin(phi);
    plane[2] = TMath::Sin(theta);
    plane[3] = cp.fDist;
    glClipPlane(GL_CLIP_PLANE0 + mClipId, plane);
    glEnable(GL_CLIP_PLANE0 + mClipId);
  }
}

void ZGlClipPlane_GL_Rnr::TurnOff(RnrDriver* rd)
{
  if(mClipId >= 0) {
    glDisable(GL_CLIP_PLANE0 + mClipId);
    rd->GL()->ReturnClipPlane(mClipId);
  }
  mClipId = -1;
}

void ZGlClipPlane_GL_Rnr::RnrSelf()
{
  ZGlClipPlane& cp = *mZGlClipPlane;

  GL_Capability_Switch light_off(GL_LIGHTING, false);

  glColor3f(1, 1, 1);
  glPushMatrix();
  glRotatef( cp.fPhi,   0, 0, 1);
  glRotatef(-cp.fTheta, 0, 1, 0);
  glTranslatef(-cp.fDist, 0, 0);
  glBegin(GL_LINE_LOOP);
  glVertex3f(0, -0.5, -0.5);
  glVertex3f(0, -0.5,  0.5);
  glVertex3f(0,  0.5,  0.5);
  glVertex3f(0,  0.5, -0.5);
  glEnd();
  glBegin(GL_LINES);
  glVertex3f(0, 0, 0); glVertex3f(-1, 0, 0);
  glEnd();
  glPopMatrix();
}
