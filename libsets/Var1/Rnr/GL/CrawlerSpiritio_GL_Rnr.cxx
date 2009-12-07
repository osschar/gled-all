// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CrawlerSpiritio_GL_Rnr.h"
#include <RnrBase/Fl_Event_Enums.h>

#include <RnrBase/RnrDriver.h>

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

  Crawler &C = * mCrawlerSpiritio->get_crawler();

  // Throttle bar
  glPushMatrix();
  glTranslatef(0.5f, 0.0f, 0.0f);
  glScalef(0.25f, 0.5f, 1.0f);
  draw_vertical_desirevar(C.RefThrottle());
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0f, -0.5f, 0.0f);
  glScalef(-0.5f, -0.22f, 1.0f); // Smaller height then width of throttle ... looks better.
  draw_horizontal_desirevar(C.RefWheel());
  glPopMatrix();


  ZTrans dyno2cam;
  dyno2cam.SetTrans(mCrawlerSpiritio->GetCamera()->RefTrans());
  dyno2cam.MultRight(*rd->GetCamFixTrans());
  dyno2cam.Invert();

  update_al_src(mCrawlerSpiritio->GetEngineSrc(), dyno2cam, rd);
  update_al_src(mCrawlerSpiritio->GetGunSrc(),    dyno2cam, rd);
}

//==============================================================================

// This not needed now ... KeyHandling is done in Spiritio_GL_Rnr.
// We pass it all the data it needs in _init().

// int CrawlerSpiritio_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
// {
//   CrawlerSpiritio &S = * mCrawlerSpiritio;
// }
