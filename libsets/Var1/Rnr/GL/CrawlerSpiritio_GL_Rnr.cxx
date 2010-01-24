// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CrawlerSpiritio_GL_Rnr.h"
#include <RnrBase/Fl_Event_Enums.h>

#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/GLRnrDriver.h>

#include <Glasses/Crawler.h>
#include <Glasses/TSPupilInfo.h> // Should notify glass, really, for FOV.
#include <Glasses/Camera.h>
#include <Glasses/AlSource.h>

#include <GL/glew.h>

//==============================================================================

void CrawlerSpiritio_GL_Rnr::_init()
{
  RegisterKey('w', "IncThrottle");
  RegisterKey('s', "DecThrottle");

  RegisterKey('q', "LeftWheel");
  RegisterKey('e', "RightWheel");

  RegisterKey(FL_Up,    "LaserUp");
  RegisterKey(FL_Down,  "LaserDown");
  RegisterKey(FL_Left,  "LaserLeft");
  RegisterKey(FL_Right, "LaserRight");

  RegisterKey(' ', "FireGun");

  RegisterKey(FL_Home, "TurretHome");
}

CrawlerSpiritio_GL_Rnr::CrawlerSpiritio_GL_Rnr(CrawlerSpiritio* idol) :
  Spiritio_GL_Rnr(idol),
  mCrawlerSpiritio(idol)
{
  _init();
}

CrawlerSpiritio_GL_Rnr::~CrawlerSpiritio_GL_Rnr()
{}

//==============================================================================

void CrawlerSpiritio_GL_Rnr::Draw(RnrDriver* rd)
{
  // Assume we are in -1, 1 mapping with origin at center.

  REF_OR_RET(Crawler, C, mCrawlerSpiritio->get_crawler());

  bool picking = rd->GL()->PickingP();

  // Throttle bar
  glPushMatrix();
  glTranslatef(0.5f, 0.0f, 0.0f);
  glScalef(0.25f, 0.5f, 1.0f);
  draw_vertical_desirevar(C.RefThrottle(), rd, picking);
  glPopMatrix();

  // Wheel bar
  glPushMatrix();
  glTranslatef(0.0f, -0.5f, 0.0f);
  glScalef(-0.5f, -0.22f, 1.0f); // Smaller height then width of throttle ... looks better.
  draw_horizontal_desirevar(C.RefWheel(), rd, picking);
  glPopMatrix();

  // LaserCharge bar
  glPushMatrix();
  glTranslatef(0.6f, 0.0f, 0.0f);
  glScalef(0.125f, 0.25f, 1.0f);
  draw_vertical_minmaxvar(C.RefLaserCharge(), rd, picking);
  glPopMatrix();

  // Laser stuff
  {
    glColor3f(1.0f, 0.0f, 0.0f);

    Float_t dd = 0.05f;
    glBegin(GL_LINE_LOOP);
    glVertex2f(-dd, -dd);
    glVertex2f( dd, -dd);
    glVertex2f( dd,  dd);
    glVertex2f(-dd,  dd);
    glEnd();

    const Float_t x0 = -2.0f * tanf(C.RefLaserLtRt().Get() - C.RefLaserLtRt().GetDesire()) * rd->GetAspect();
    const Float_t y0 =  2.0f * tanf(C.RefLaserUpDn().Get() - C.RefLaserUpDn().GetDesire());
    dd = 0.04f;
    glBegin(GL_LINES);
    glVertex2f(x0, y0 + dd);
    glVertex2f(x0, y0 - dd);
    glVertex2f(x0 - dd, y0);
    glVertex2f(x0 + dd, y0);
    glEnd();
  }

  ZTrans dyno2cam;
  dyno2cam.SetTrans(mCrawlerSpiritio->GetCamera()->RefTrans());
  dyno2cam.MultRight(*rd->GetCamFixTrans());
  dyno2cam.Invert();

  update_al_src(mCrawlerSpiritio->GetEngineSrc(), dyno2cam, rd);
}

//==============================================================================

int CrawlerSpiritio_GL_Rnr::HandleMouse(RnrDriver* rd, Fl_Event& ev)
{
  // Prevents further mouse interaction.

  CrawlerSpiritio &S = * mCrawlerSpiritio;

  REF_OR_RET_VAL(Crawler, C, S.get_crawler(), 0);

  if (ev.fIsOverlay)
  {
    if (ev.fEvent == FL_PUSH || ev.fEvent == FL_DRAG)
    {
      Float_t norm_z = 0.25f * (ev.fZMin + ev.fZMax);

      if (ev.fCurrentNSE->fUserData == &C.RefThrottle())
      {
	const SDesireVarF &d = C.RefThrottle();
	S.SetThrottle(d.GetMin() + norm_z * (d.GetMax() - d.GetMin()));
	return 1;
      }
      else if (ev.fCurrentNSE->fUserData == &C.RefWheel())
      {
	const SDesireVarF &d = C.RefWheel();
	S.SetWheel(d.GetMin() + norm_z * (d.GetMax() - d.GetMin()));
	return 1;
      }
    }
  }
  else
  {
    if (ev.fEvent == FL_MOUSEWHEEL)
    {
      Float_t delta = -5.0f * ev.fDY;
      Float_t ex_fov = S.GetPupilInfo()->GetZFov();
      if (delta > 0)
      {
	if (ex_fov < 120.0f)
	  S.GetPupilInfo()->SetZFov(TMath::Min(120.0f, ex_fov+delta));
      }
      else
      {
	if (ex_fov > 30.0f)
	  S.GetPupilInfo()->SetZFov(TMath::Max(30.0f, ex_fov+delta));
      }
    }
  }

  return 1;
}
