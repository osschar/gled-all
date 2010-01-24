// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Spiritio_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <Rnr/GL/GLRnrDriver.h>
#include <RnrBase/Fl_Event_Enums.h>
#include <Glasses/AlSource.h>
#include <Eye/Eye.h>

#include <GL/glew.h>

//==============================================================================

void Spiritio_GL_Rnr::_init()
{}

Spiritio_GL_Rnr::Spiritio_GL_Rnr(Spiritio* idol) :
  ZNode_GL_Rnr(idol),
  mSpiritio(idol)
{
  _init();
}

Spiritio_GL_Rnr::~Spiritio_GL_Rnr()
{}

//==============================================================================

void Spiritio_GL_Rnr::AbsorbRay(Ray& ray)
{
  if (ray.fRQN == Spiritio::PRQN_release_keys)
  {
    for (mK2KIRep_i i = mKeyReps.begin(); i != mKeyReps.end(); ++i)
    {
      if (i->second.fIsDown)
	i->second.fIsDown = false;
    }
    return;
  }

  ZNode_GL_Rnr::AbsorbRay(ray);
}

//==============================================================================

int Spiritio_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  if (ev.fEvent == FL_LEAVE && ! ev.fIsOverlay)
  {
    auto_ptr<ZMIR> mir(mSpiritio->S_ReleaseAllKeys());
    fImg->fEye->Send(*mir);
    return 1;
  }

  if ( ! mSpiritio->bActive)
  {
    return 0;
  }

  if (ev.fIsKey)
  {
    return HandleKey(rd, ev);
  }
  else if (ev.fIsMouse)
  {
    return HandleMouse(rd, ev);
  }
  else
  {
    return 0;
  }
}

int Spiritio_GL_Rnr::HandleKey(RnrDriver* rd, Fl_Event& ev)
{
  // Here we always call registered functions directly.
  // Alternatively, we could send MIRs.

  Spiritio &S = * mSpiritio;

  int non_handled = S.mEatNonBoundKeyEvents ? 1 : 0;

  if (ev.fEvent == FL_KEYDOWN)
  {
    mK2KIRep_i i = mKeyReps.find(ev.fKey);
    if (i == mKeyReps.end())
      return non_handled;

    if( ! i->second.fIsDown)
    {
      GTime now(GTime::I_Now);
      UInt_t elapsed = (now - i->second.fLastTime).ToMiliSec(10);

      // printf("Key down, idx=%d, elapsed=%u\n", i->second.fIndex, elapsed);

      S.HandleKey(i->second.fIndex, true, elapsed);

      i->second.fIsDown   = true;
      i->second.fLastTime = now;
    }
    return 1;
  }
  else if (ev.fEvent == FL_KEYUP)
  {
    mK2KIRep_i i = mKeyReps.find(ev.fKey);
    if (i == mKeyReps.end())
      return non_handled;

    if (i->second.fIsDown)
    {
      GTime now(GTime::I_Now);
      UInt_t elapsed = (now - i->second.fLastTime).ToMiliSec(10);

      // printf("keyup, idx=%d, elapsed=%u\n", i->second.fIndex, elapsed);

      S.HandleKey(i->second.fIndex, false, elapsed);

      i->second.fIsDown = false;
      // Do not store last up key, for doble-click need dt between two
      // button-downs.
      // i->second.fLastTime = now;
    }
    return 1;
  }
  else
  {
    assert(false);
    return 0;
  }
}

//==============================================================================

void Spiritio_GL_Rnr::RegisterKey(Int_t k, const TString& tag)
{
  static const Exc_t _eh("Spiritio_GL_Rnr::RegisterKey ");

  Int_t idx = mSpiritio->FindKey(tag);
  if (idx != -1)
    mKeyReps.insert(make_pair(k, KeyHandling::KeyInfoRep(idx)));
  else
    ISwarn(_eh + "tag '" + tag + "' requested for key '" + char(k) + "' not found.");
}


//==============================================================================
// Functions for drawing overlay valuators
//==============================================================================

void Spiritio_GL_Rnr::draw_vertical_outline_value_quads
  (Float_t bar_hw, Float_t yzero, Float_t yval)
{
  glEnable(GL_POLYGON_OFFSET_FILL);

  glPolygonOffset(4, 4);
  glColor4f(0.8, 0.8, 0.8, 0.4);
  glBegin(GL_QUADS);
  glVertex2f(-bar_hw, -0.5f); glVertex2f( bar_hw, -0.5f);
  glVertex2f( bar_hw,  0.5f); glVertex2f(-bar_hw,  0.5f);
  glEnd();

  glPolygonOffset(2, 2);
  glColor4f(0.8, 0.4, 0.4, 0.8);
  glBegin(GL_QUADS);
  if (yval >= yzero)
  {
    glVertex2f(-bar_hw, yzero); glVertex2f( bar_hw, yzero);
    glVertex2f( bar_hw, yval);  glVertex2f(-bar_hw, yval);
  }
  else
  {
    glVertex2f(-bar_hw, yval);  glVertex2f( bar_hw, yval);
    glVertex2f( bar_hw, yzero); glVertex2f(-bar_hw, yzero);
  }
  glEnd();

  glDisable(GL_POLYGON_OFFSET_FILL);
}

void Spiritio_GL_Rnr::draw_horizontal_outline_value_quads
  (Float_t bar_hh, Float_t xzero, Float_t xval)
{
  glEnable(GL_POLYGON_OFFSET_FILL);

  glPolygonOffset(4, 4);
  glColor4f(0.8, 0.8, 0.8, 0.4);
  glBegin(GL_QUADS);
  glVertex2f(-0.5f,  bar_hh); glVertex2f(-0.5f, -bar_hh);
  glVertex2f( 0.5f, -bar_hh); glVertex2f( 0.5f,  bar_hh);
  glEnd();

  glPolygonOffset(2, 2);
  glColor4f(0.8, 0.4, 0.4, 0.8);
  glBegin(GL_QUADS);
  if (xval >= xzero)
  {
    glVertex2f(xzero, bar_hh); glVertex2f(xzero, -bar_hh);
    glVertex2f(xval, -bar_hh); glVertex2f(xval,   bar_hh);
  }
  else
  {
    glVertex2f(xval,   bar_hh); glVertex2f(xval, -bar_hh);
    glVertex2f(xzero, -bar_hh); glVertex2f(xzero, bar_hh);
  }
  glEnd();

  glDisable(GL_POLYGON_OFFSET_FILL);
}

//------------------------------------------------------------------------------

void Spiritio_GL_Rnr::draw_vertical_picking_quad
  (RnrDriver* rd, void* user_data, Float_t bar_hw)
{
  rd->GL()->PushName(this, user_data);
  glBegin(GL_QUADS);
  glVertex3f(-bar_hw, -0.5f,  0.0f); glVertex3f( bar_hw, -0.5f,  0.0f);
  glVertex3f( bar_hw,  0.5f, -1.0f); glVertex3f(-bar_hw,  0.5f, -1.0f);
  glEnd();
  rd->GL()->PopName();

}

void Spiritio_GL_Rnr::draw_horizontal_picking_quad
  (RnrDriver* rd, void* user_data, Float_t bar_hh)
{
  rd->GL()->PushName(this, user_data);
  glBegin(GL_QUADS);
  glVertex3f(-0.5f,  bar_hh,  0.0f); glVertex3f(-0.5f, -bar_hh,  0.0f);
  glVertex3f( 0.5f, -bar_hh, -1.0f); glVertex3f( 0.5f,  bar_hh, -1.0f);
  glEnd();
  rd->GL()->PopName();
}

//------------------------------------------------------------------------------

void Spiritio_GL_Rnr::draw_vertical_minmaxvar(const SMinMaxVarF& v, RnrDriver* rd, bool pick)
{
  // Draws into unit box x(-0.5, 0.5), y (-0.5, 0.5).  
  // Blending should be enabled from outside.
  //
  // If pick is true an invisible, z-tilted polygon is also drawn so that
  // the picking position can deduced from the max depth od the selection hit.
  // The z-offset at the bottom is 0 and -1 at the top which translates
  // to depth buffer values of 0 and 2 respectively.

  static const Float_t bar_hw  = 0.1f;  // bar half-width
  static const Float_t line_hw = 0.25f; // line half-width for val / des marks

  if (pick)
  {
    draw_vertical_picking_quad(rd, (void*) &v, bar_hw);
    return;
  }

  const Float_t min = v.GetMin(), max = v.GetMax();
  const Float_t val = v.Get();

  const Float_t dinv = 1.0f / (max - min);

  const Float_t yzero = -0.5f - min * dinv;
  const Float_t yval  = -0.5f + (val - min) * dinv;

  draw_vertical_outline_value_quads(bar_hw, yzero, yval);

  glLineWidth(4);
  glColor4f(0.4, 0.4, 0.8, 1.0);
  glBegin(GL_LINES);
  glVertex2f(-bar_hw,  yval);
  glVertex2f( line_hw, yval);  
  glEnd();

  glLineWidth(1);
  glColor4f(1.0, 0.4, 0.4, 1.0);
  glBegin(GL_LINE_LOOP);
  glVertex2f(-bar_hw, -0.5f); glVertex2f( bar_hw, -0.5f);
  glVertex2f( bar_hw,  0.5f); glVertex2f(-bar_hw,  0.5f);
  glEnd();
}

void Spiritio_GL_Rnr::draw_horizontal_minmaxvar(const SMinMaxVarF& v, RnrDriver* rd, bool pick)
{
  // Draws into unit box x(-0.5, 0.5), y (-0.5, 0.5).
  //
  // See comments in draw_vertical_desirevar.

  static const Float_t bar_hh  = 0.1f;  // bar half-width
  static const Float_t line_hh = 0.25f; // line half-width for val / des marks

  if (pick)
  {
    draw_horizontal_picking_quad(rd, (void*) &v, bar_hh);
    return;
  }

  const Float_t min = v.GetMin(), max = v.GetMax();
  const Float_t val = v.Get();

  const Float_t dinv = 1.0f / (max - min);

  const Float_t xzero = -0.5f - min * dinv;
  const Float_t xval  = -0.5f + (val - min) * dinv;

  draw_horizontal_outline_value_quads(bar_hh, xzero, xval);

  glLineWidth(4);
  glColor4f(0.4, 0.4, 0.8, 1.0);
  glBegin(GL_LINES);
  glVertex2f(xval, -bar_hh);
  glVertex2f(xval,  line_hh);  
  glEnd();

  glLineWidth(1);
  glColor4f(1.0, 0.4, 0.4, 1.0);
  glBegin(GL_LINE_LOOP);
  glVertex2f(-0.5f,  bar_hh); glVertex2f(-0.5f, -bar_hh);
  glVertex2f( 0.5f, -bar_hh); glVertex2f( 0.5f,  bar_hh);
  glEnd();
}

//==============================================================================

void Spiritio_GL_Rnr::draw_vertical_desirevar(const SDesireVarF& v, RnrDriver* rd, bool pick)
{
  // Draws into unit box x(-0.5, 0.5), y (-0.5, 0.5).  
  // Blending should be enabled from outside.
  //
  // If pick is true an invisible, z-tilted polygon is also drawn so that
  // the picking position can deduced from the max depth od the selection hit.
  // The z-offset at the bottom is 0 and -1 at the top which translates
  // to depth buffer values of 0 and 2 respectively.

  static const Float_t bar_hw  = 0.1f;  // bar half-width
  static const Float_t line_hw = 0.25f; // line half-width for val / des marks

  if (pick)
  {
    draw_vertical_picking_quad(rd, (void*) &v, bar_hw);
    return;
  }

  const Float_t min = v.GetMin(), max = v.GetMax();
  const Float_t val = v.Get(),    des = v.GetDesire();

  const Float_t dinv = 1.0f / (max - min);

  const Float_t yzero = -0.5f - min * dinv;
  const Float_t yval  = -0.5f + (val - min) * dinv;
  const Float_t ydes  = -0.5f + (des - min) * dinv;

  draw_vertical_outline_value_quads(bar_hw, yzero, yval);

  glLineWidth(2);
  glColor4f(0.4, 0.8, 0.4, 1.0);
  glBegin(GL_LINES);
  glVertex2f(-line_hw, ydes);
  glVertex2f( bar_hw,  ydes);
  glEnd();

  glLineWidth(4);
  glColor4f(0.4, 0.4, 0.8, 1.0);
  glBegin(GL_LINES);
  glVertex2f(-bar_hw,  yval);
  glVertex2f( line_hw, yval);  
  glEnd();

  glLineWidth(1);
  glColor4f(1.0, 0.4, 0.4, 1.0);
  glBegin(GL_LINE_LOOP);
  glVertex2f(-bar_hw, -0.5f); glVertex2f( bar_hw, -0.5f);
  glVertex2f( bar_hw,  0.5f); glVertex2f(-bar_hw,  0.5f);
  glEnd();
}

void Spiritio_GL_Rnr::draw_horizontal_desirevar(const SDesireVarF& v, RnrDriver* rd, bool pick)
{
  // Draws into unit box x(-0.5, 0.5), y (-0.5, 0.5).
  //
  // See comments in draw_vertical_desirevar.

  static const Float_t bar_hh  = 0.1f;  // bar half-width
  static const Float_t line_hh = 0.25f; // line half-width for val / des marks

  if (pick)
  {
    draw_horizontal_picking_quad(rd, (void*) &v, bar_hh);
    return;
  }

  const Float_t min = v.GetMin(), max = v.GetMax();
  const Float_t val = v.Get(),    des = v.GetDesire();

  const Float_t dinv = 1.0f / (max - min);

  const Float_t xzero = -0.5f - min * dinv;
  const Float_t xval  = -0.5f + (val - min) * dinv;
  const Float_t xdes  = -0.5f + (des - min) * dinv;

  draw_horizontal_outline_value_quads(bar_hh, xzero, xval);

  glLineWidth(2);
  glColor4f(0.4, 0.8, 0.4, 1.0);
  glBegin(GL_LINES);
  glVertex2f(xdes, -line_hh);
  glVertex2f(xdes,  bar_hh);
  glEnd();

  glLineWidth(4);
  glColor4f(0.4, 0.4, 0.8, 1.0);
  glBegin(GL_LINES);
  glVertex2f(xval, -bar_hh);
  glVertex2f(xval,  line_hh);  
  glEnd();

  glLineWidth(1);
  glColor4f(1.0, 0.4, 0.4, 1.0);
  glBegin(GL_LINE_LOOP);
  glVertex2f(-0.5f,  bar_hh); glVertex2f(-0.5f, -bar_hh);
  glVertex2f( 0.5f, -bar_hh); glVertex2f( 0.5f,  bar_hh);
  glEnd();
}

//==============================================================================

void Spiritio_GL_Rnr::update_al_src(AlSource* src, const ZTrans& t, RnrDriver* rd)
{
  if (src)
  {
    src->ref_trans().SetTrans(t);
    src->MarkStampReqTrans();
    rd->Render(rd->GetLensRnr(src));
  }
}
