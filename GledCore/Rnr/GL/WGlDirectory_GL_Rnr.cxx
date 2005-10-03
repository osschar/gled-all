// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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

#include <GL/gl.h>

/**************************************************************************/

void WGlDirectory_GL_Rnr::_init()
{
  // From ZGlass_GL_Rnr
  bUsesSubPicking = true;

  mCurrent = 0;
}

/**************************************************************************/

//void WGlDirectory_GL_Rnr::PreDraw(RnrDriver* rd)
//{}

void WGlDirectory_GL_Rnr::Draw(RnrDriver* rd)
{
  WGlDirectory& M = *mWGlDirectory;
  if(M.mContents == 0 || M.mContents->IsEmpty())
    return;

  obtain_rnrmod(rd, mFontRMS);
  obtain_rnrmod(rd, mFrameRMS);

  GLTextNS::TexFont    *txf = ((ZRlFont_GL_Rnr*)mFontRMS.rnr())->GetFont();
  WGlFrameStyle_GL_Rnr &FSR = *(WGlFrameStyle_GL_Rnr*)mFrameRMS.rnr();

  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POLYGON_BIT);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  lpZGlass_t cont; M.mContents->CopyList(cont);

  SGridStepper stepper(M.mStepMode);
  stepper.SetNs(M.mNx, M.mNy, M.mNz);
  stepper.SetDs(M.mDx, M.mDy, M.mDz);

  GledNS::ClassInfo* bci = M.GetCbackBetaClassInfo();

  for(lpZGlass_i i=cont.begin(); i!=cont.end(); ++i) {

    if(bci && ! GledNS::IsA(*i, bci->fFid))
      continue;

    TString lens_name((*i)->GetName());

    ZColor back_color = M.mBoxColor;
    // if(*i selected)
    //  back_color = M.mBoxColor();
    bool belowmouse = (*i == mCurrent);

    glPushMatrix();
    Float_t pos[3];
    stepper.GetPosition(pos);
    glTranslatef(pos[0], pos[1], pos[2]);
    rd->GL()->PushName(this, *i);

    if(M.bDrawBox) {
      glColor4fv(back_color());
      glPushMatrix();
      glTranslatef(M.mBoxOx, M.mBoxOy, M.mBoxOz);
      glScalef(M.mBoxDx, M.mBoxDy, M.mBoxDz);
      SphereTrings::UnitBox();
      glPopMatrix();
    }

    if(M.bDrawText) {
      glPushMatrix();
      glTranslatef(M.mTextOx, M.mTextOy, M.mTextOz);
      glRotatef( M.mTextA1, 0, 0, 1);
      glRotatef(-M.mTextA2, 0, 1, 0);
      glRotatef( M.mTextA3, 1, 0, 0);
      FSR.FullRender(txf, lens_name, M.mTextDx, M.mTextDy, belowmouse);
      glPopMatrix();
    }

    rd->GL()->PopName();
    glPopMatrix();

    if(stepper.Step() == false)
      printf("out of space ... proceeding any way\n");
  }

  glPopAttrib();
}

//void WGlDirectory_GL_Rnr::PostDraw(RnrDriver* rd)
//{}

/**************************************************************************/

int WGlDirectory_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  // printf("Handloo: %d %d 0x%x 0x%x %p '%s'\n", ev.fEvent,
  // ev.fKey, ev.fButton, ev.fState, ev.fBelowMouse, 
  // ev.fBelowMouse ? ev.fBelowMouse->fLens->GetName() : "-");

  WGlDirectory& M = *mWGlDirectory;

  if(ev.fEvent == FL_LEAVE) {
    if(mCurrent) {
      mCurrent = 0;
      Redraw(rd);
    }
    return 1;
  }
  
  if(ev.fEvent == FL_ENTER || ev.fEvent == FL_MOVE) {
    ZGlass* tgt = (ZGlass*) ev.fCurrentNSE->fUserData;
    if(tgt != mCurrent) {
      mCurrent = tgt;
      Redraw(rd);
    }
    return 1;
  }

  if(ev.fEvent == FL_DRAG) {
    ZGlass* tgt = (ZGlass*) ev.fCurrentNSE->fUserData;
    if(tgt != mCurrent) {
      mCurrent = 0;
      Redraw(rd);
    }
    return 1;
  }

  if(ev.fEvent == FL_PUSH && ev.fButton == FL_LEFT_MOUSE) {
    if(mCurrent) {
      GledNS::MethodInfo* mi = M.GetCbackMethodInfo();
      if(mi == 0) return 0;
      ZMIR mir(M.mCbackAlpha.get(), mCurrent);
      mi->ImprintMir(mir);
      fImg->fEye->Send(mir);

      mCurrent = 0;
      Redraw(rd);
    }
    return 1;
  }

  return 0;
}
