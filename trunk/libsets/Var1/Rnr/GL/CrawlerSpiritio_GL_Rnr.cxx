// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CrawlerSpiritio_GL_Rnr.h"
#include <RnrBase/Fl_Event_Enums.h>

#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/GLRnrDriver.h>

#include <Glasses/Crawler.h>
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

  RegisterKey(' ', "FireGun");
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


  ZTrans dyno2cam;
  dyno2cam.SetTrans(mCrawlerSpiritio->GetCamera()->RefTrans());
  dyno2cam.MultRight(*rd->GetCamFixTrans());
  dyno2cam.Invert();

  update_al_src(mCrawlerSpiritio->GetEngineSrc(), dyno2cam, rd);
}

//==============================================================================

int CrawlerSpiritio_GL_Rnr::HandleMouse(RnrDriver* rd, Fl_Event& ev)
{
  // If not called from handle_overlay do nothing for now.
  if (!ev.fIsOverlay)
    return 0;

  CrawlerSpiritio &S = * mCrawlerSpiritio;

  REF_OR_RET_VAL(Crawler, C, S.get_crawler(), 0);

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

  return 0;
}
