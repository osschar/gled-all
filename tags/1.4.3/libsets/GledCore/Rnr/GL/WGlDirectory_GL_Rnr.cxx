// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "WGlDirectory_GL_Rnr.h"

#include <RnrBase/Fl_Event_Enums.h>
#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/SphereTrings.h>
#include <Rnr/GL/ZRlFont_GL_Rnr.h>
#include <Rnr/GL/WGlFrameStyle_GL_Rnr.h>

#include <Stones/SGridStepper.h>

#include <Stones/ZMIR.h>
#include <Eye/Eye.h>

#include <GL/glew.h>

//------------------------------------------------------------------------------

void WGlDirectory_GL_Rnr::_init()
{
  // From ZGlass_GL_Rnr
  bUsesSubPicking = true;

  m_current   = 0;
  m_prev_page = &m_prev_page;
  m_next_page = &m_next_page;
  m_prev = &m_prev;
  m_next = &m_next;
}

//------------------------------------------------------------------------------

// void WGlDirectory_GL_Rnr::PreDraw(RnrDriver* rd) {}

void WGlDirectory_GL_Rnr::Draw(RnrDriver* rd)
{
  WGlDirectory& M = *mWGlDirectory;
  if ((M.bDrawPageCtrl == false) && (M.mContents == 0 || M.mContents->IsEmpty()))
    return;

  obtain_rnrmod(rd, mFontRMS);
  obtain_rnrmod(rd, mFrameRMS);

  FTFont *txf = ((ZRlFont_GL_Rnr*)mFontRMS.rnr())->GetFont();
  WGlFrameStyle_GL_Rnr &FSR = *(WGlFrameStyle_GL_Rnr*)mFrameRMS.rnr();

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  SGridStepper stepper(M.mStepMode);
  stepper.SetNs(M.mNx, M.mNy, M.mNz);
  stepper.SetDs(M.mDx, M.mDy, M.mDz);

  int all_count = M.count_entries();
  int first     = M.mFirst;
  int count     = 0;

  GledNS::ClassInfo* bci = M.GetCbackBetaClassInfo();
  lpZGlass_t cont; M.mContents->CopyList(cont);
  for (lpZGlass_i i=cont.begin(); i!=cont.end(); ++i)
  {
    if (bci && ! GledNS::IsA(*i, bci->fFid)) continue;
    count ++;
    if (count < first) continue;

    ZColor back_color = M.mBoxColor;
    bool belowmouse = (*i == m_current);

    glPushMatrix();
    Float_t pos[3];
    stepper.GetPosition(pos);
    glTranslatef(pos[0], pos[1], pos[2]);
    rd->GL()->PushName(this, *i);

    if (M.bDrawBox)
    {
      glColor4fv(back_color());
      glPushMatrix();
      glTranslatef(M.mBoxOx, M.mBoxOy, M.mBoxOz);
      glScalef(M.mBoxDx, M.mBoxDy, M.mBoxDz);
      SphereTrings::UnitBox();
      glPopMatrix();
    }

    if (M.bDrawText)
    {
      TString lens_name((*i)->GetName());
      if (M.bDrawTitle)
      {
	// name
	Float_t ntw = M.mTextDx*M.mNameFraction;
	glPushMatrix();
	glTranslatef(M.mTextOx, M.mTextOy, M.mTextOz);
	glRotatef( M.mTextA1, 0, 0, 1);
	glRotatef(-M.mTextA2, 0, 1, 0);
	glRotatef( M.mTextA3, 1, 0, 0);
	FSR.FullRender(txf, lens_name, ntw, M.mTextDy, belowmouse);
	glPopMatrix();
	// title
	glPushMatrix();
	glTranslatef( ntw + M.mTextOx, M.mTextOy, M.mTextOz);
	glRotatef( M.mTextA1, 0, 0, 1);
	glRotatef(-M.mTextA2, 0, 1, 0);
	glRotatef( M.mTextA3, 1, 0, 0);
	TString lens_title((*i)->GetTitle());
	FSR.FullRender(txf, lens_title,  M.mTextDx-ntw, M.mTextDy, belowmouse);
	glPopMatrix();
      }
      else
      {
	// name
	glPushMatrix();
	glTranslatef(M.mTextOx, M.mTextOy, M.mTextOz);
	glRotatef( M.mTextA1, 0, 0, 1);
	glRotatef(-M.mTextA2, 0, 1, 0);
	glRotatef( M.mTextA3, 1, 0, 0);
	FSR.FullRender(txf, lens_name, M.mTextDx, M.mTextDy, belowmouse);
	glPopMatrix();
      }
    }
    rd->GL()->PopName();
    glPopMatrix();

    if(stepper.Step() == false)
      break;
  }


  if (M.bDrawPageCtrl)
  {
    ZColor ptc = M.mPageColor;
    ZColor psc = M.mSymColor;
    glPushMatrix();
    glTranslatef(0, M.mNy*M.mDy, 0);
    float pcw =  M.mPageCtrlWidth*M.mTextDx;
    float piw =  M.mPageInfoWidth*M.mTextDx;
    float pco =  M.mPageCtrlOff*M.mTextDx;
    float pio =  M.mPageInfoOff*M.mTextDx;

    rd->GL()->PushName(this, m_prev_page);
    FSR.FullSymbolRender(txf, "<<", pcw, M.mTextDy, m_current == m_prev_page, &ptc, &psc);
    rd->GL()->PopName();

    glTranslatef(pcw + pco, 0, 0);
    rd->GL()->PushName(this, m_prev);
    FSR.FullSymbolRender(txf, "<", pcw, M.mTextDy, m_current == m_prev, &ptc, &psc);
    rd->GL()->PopName();

    glTranslatef(pcw + pio, 0, 0);
    rd->GL()->PushName(this, 0);
    FSR.FullSymbolRender(txf,  GForm("%d/%d", M.mFirst, all_count), piw, M.mTextDy, false, &ptc);
    rd->GL()->PopName();

    glTranslatef(piw + pio, 0, 0);
    rd->GL()->PushName(this, m_next);
    FSR.FullSymbolRender(txf, ">", pcw, M.mTextDy, m_current == m_next, &ptc, &psc);
    rd->GL()->PopName();

    glTranslatef(pcw + pco, 0, 0);
    rd->GL()->PushName(this, m_next_page);
    FSR.FullSymbolRender(txf, ">>", pcw, M.mTextDy, m_current == m_next_page, &ptc, &psc);
    rd->GL()->PopName();
    glPopMatrix();
  }

  glPopAttrib();
}

// void WGlDirectory_GL_Rnr::PostDraw(RnrDriver* rd) {}

//------------------------------------------------------------------------------

int WGlDirectory_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  // printf("Handloo: %d %d 0x%x 0x%x %p '%s'\n", ev.fEvent,
  // ev.fKey, ev.fButton, ev.fState, ev.fBelowMouse,
  // ev.fBelowMouse ? ev.fBelowMouse->fLens->GetName() : "-");

  WGlDirectory& M = *mWGlDirectory;

  if (ev.fEvent == FL_LEAVE)
  {
    if (m_current) {
      m_current = 0;
      Redraw(rd);
    }
    return 1;
  }

  if (ev.fEvent == FL_ENTER || ev.fEvent == FL_MOVE)
  {
    if (ev.fCurrentNSE->fUserData != m_current) {
      m_current = ev.fCurrentNSE->fUserData;
      Redraw(rd);
    }
    return 1;
  }

  if (ev.fEvent == FL_DRAG)
  {
    if (ev.fCurrentNSE->fUserData != m_current) {
      m_current = ev.fCurrentNSE->fUserData;
      Redraw(rd);
    }
    return 1;
  }

  if (ev.fEvent == FL_PUSH && ev.fButton == FL_LEFT_MOUSE)
  {
    if (m_current == m_prev_page) {
      auto_ptr<ZMIR> mir( M.S_PrevPage() );
      fImg->fEye->Send(*mir);
    } else if (m_current == m_next_page) {
      auto_ptr<ZMIR> mir( M.S_NextPage() );
      fImg->fEye->Send(*mir);
    } else if (m_current == m_next) {
      auto_ptr<ZMIR> mir( M.S_Next() );
      fImg->fEye->Send(*mir);
    } else if (m_current == m_prev) {
      auto_ptr<ZMIR> mir( M.S_Prev() );
      fImg->fEye->Send(*mir);
    } else if (m_current != 0) {
      ZGlass* lens = (ZGlass*)m_current;
      auto_ptr<ZMIR> clicked( M.S_SetLastClicked(lens) );
      fImg->fEye->Send(*clicked);
      GledNS::MethodInfo* mi = M.GetCbackMethodInfo();
      if (mi == 0) return 0;
      ZMIR cback(M.mCbackAlpha.get(), lens);
      mi->ImprintMir(cback);
      fImg->fEye->Send(cback);
    }
    m_current = 0;
    Redraw(rd);
    return 1;
  }

  return 0;
}

void WGlDirectory_GL_Rnr::HandlePick(RnrDriver* rd, lNSE_t& ns, lNSE_i nsi)
{
  // Only here as demonstration: trace pick event delivery.

  static const Exc_t _eh("WGlDirectory_GL_Rnr::HandlePick ");

  // It can be called on WGlDirectory itself, user-data of NameStackEntry
  // will be 0 in this case.
  // Otherwise it is one of the dependant items, as pushed on namestack in
  // Draw().
  // There can, of course, also be several relevant entries in the namestack.
  // Use iterator++ to move on.
  // NOTE: Order in the list is *reversed*, as returned by GL selection.

  if (nsi->fUserData == 0)
    printf("%sDeliver to WGlDirectory itself.\n", _eh.Data());
  else if (nsi->fUserData == m_prev_page)
    printf("%sDeliver to m_prev_page.\n", _eh.Data());
  else if (nsi->fUserData == m_next_page)
    printf("%sDeliver to m_next_page.\n", _eh.Data());
  else if (nsi->fUserData == m_prev)
    printf("%sDeliver to m_prev.\n", _eh.Data());
  else if (nsi->fUserData == m_next)
    printf("%sDeliver to m_next.\n", _eh.Data());
  else {
    ZGlass* lens = (ZGlass*) nsi->fUserData;
    printf("%sDeliver to dir entry: '%s'.\n", _eh.Data(), lens->GetName());
  }
}
