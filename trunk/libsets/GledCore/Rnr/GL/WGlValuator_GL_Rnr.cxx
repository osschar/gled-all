// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "WGlValuator_GL_Rnr.h"

#include <RnrBase/Fl_Event_Enums.h>
#include <Rnr/GL/ZRlFont_GL_Rnr.h>
#include <Rnr/GL/WGlFrameStyle_GL_Rnr.h>

#include <Stones/ZMIR.h>
#include <Eye/Eye.h>

#include <TDataMember.h>
#include <TDataType.h>

#include <GL/glew.h>

/**************************************************************************/

void WGlValuator_GL_Rnr::_init()
{
  bBelowMouse = 0;
}

/**************************************************************************/

Double_t WGlValuator_GL_Rnr::get_value()
{
  WGlValuator& V = *mWGlValuator;
  char* addr = (char*)*V.mCbackAlpha + V.mDataMember->GetOffset();
  return GledNS::peek_value(addr, V.mDataMember->GetDataType()->GetType());
}

Bool_t WGlValuator_GL_Rnr::send_value(Double_t step_base)
{
  // Returns: true if MIT was sent, false otherwise.

  WGlValuator& V = *mWGlValuator;

  if (V.DataOK() == false || V.GetConstVal())
    return false;

  ZMIR mir(*V.mCbackAlpha);
  V.mDataMemberInfo->fSetMethod->ImprintMir(mir);
  Double_t      val  = step_base*V.mStepA;
  if (V.mStepB) val /= V.mStepB;
  val += get_value();
  if (val > V.mMax) val = V.mMax;
  if (val < V.mMin) val = V.mMin;
  GledNS::stream_value(mir, V.mDataMember->GetDataType()->GetType(), val);
  fImg->fEye->Send(mir);

  return true;
}

/**************************************************************************/

void WGlValuator_GL_Rnr::Draw(RnrDriver* rd)
{
  obtain_rnrmod(rd, mFontRMS);
  obtain_rnrmod(rd, mFrameRMS);

  GLTextNS::TexFont    *txf = ((ZRlFont_GL_Rnr*)mFontRMS.rnr())->GetFont();
  WGlFrameStyle_GL_Rnr &FSR = *(WGlFrameStyle_GL_Rnr*)mFrameRMS.rnr();

  WGlValuator& V = *mWGlValuator;

  TString label("<no-set>");
  if(V.DataOK())
    label = GForm(V.GetFormat(), get_value());
  FSR.FullRender(txf, label, V.mDx, V.mDy, bBelowMouse);
}

/**************************************************************************/

int WGlValuator_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  if(ev.fEvent == FL_LEAVE) {
    bBelowMouse = false;
    Redraw(rd);
    return 1;
  }

  if(ev.fEvent == FL_ENTER) {
    bBelowMouse = true;
    Redraw(rd);
    return 1;
  }

  if(ev.fEvent == FL_PUSH) {
    switch (ev.fButton)
    {
      case FL_LEFT_MOUSE:   mButtFac = 1;   break;
      case FL_MIDDLE_MOUSE: mButtFac = 10;  break;
      case FL_RIGHT_MOUSE:  mButtFac = 100; break;
    }
    mX = ev.fX; mY = ev.fY;
    // printf("push at %d %d\n", mX, mY);
    return 1;
  }

  if(ev.fEvent == FL_DRAG) {
    Int_t dx = (ev.fX - mX) / 4;
    // printf("drag at %d %d; x=%d, d=%d\n", ev.fX, ev.fX, mX, dx);
    if (dx != 0)
    {
      send_value(dx*mButtFac);
      mX = ev.fX;
    }
    return 1;
  }
  /*
    if(ev.fEvent == FL_RELEASE) {
    bBelowMouse = true;
    Redraw(rd);
    return 1;
    }
  */
  if(ev.fEvent == FL_MOUSEWHEEL) {
    if(ev.fDY) {
      Double_t step = -ev.fDY;
      if (ev.fState & FL_SHIFT) {
        step *= 10;
        if (ev.fState & FL_CTRL)
          step *= 10;
      }
      send_value(step);
    }
    return 1;
  }

  return 0;
}
