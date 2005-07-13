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

  lpZGlass_t cont; M.mContents->Copy(cont);

  Int_t nx=0, ny=0, nz=0;
  Int_t *ls[3], *ns[3];
  switch(M.mStepMode) { 
  case WGlDirectory::SM_XYZ:
    ls[0] = &M.mNx; ls[1] = &M.mNy; ls[2] = &M.mNz;
    ns[0] = &nx;    ns[1] = &ny;    ns[2] = &nz;
    break;
  case WGlDirectory::SM_YXZ:
    ls[0] = &M.mNy; ls[1] = &M.mNx; ls[2] = &M.mNz;
    ns[0] = &ny;    ns[1] = &nx;    ns[2] = &nz;
    break;
  case WGlDirectory::SM_XZY:
    ls[0] = &M.mNx; ls[1] = &M.mNz; ls[2] = &M.mNy;
    ns[0] = &nx;    ns[1] = &nz;    ns[2] = &ny;
    break;
  }

  GledNS::ClassInfo* bci = M.GetCbackBetaClassInfo();

  for(lpZGlass_i i=cont.begin(); i!=cont.end(); ++i) {

    if(bci && ! GledNS::IsA(*i, bci->fFid))
      continue;

    string lens_name((*i)->GetName());

    ZColor back_color = M.mBoxColor;
    // if(*i selected)
    //  back_color = M.mBoxColor();
    bool belowmouse = (*i == mCurrent);

    glPushMatrix();
    glTranslatef(nx*M.mDx, ny*M.mDy, nz*M.mDz);
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

    (*ns[0])++;
    if(*ns[0] >= *ls[0]) {
      *ns[0] = 0; (*ns[1])++;
      if(*ns[1] >= *ls[1]) {
	*ns[1] = 0; (*ns[2])++;
	if(*ns[2] >= *ls[2]) {
	  printf("out of space ... proceeding any way\n");
	}
      }
    }
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
  // ev.fBelowMouse ? ev.fBelowMouse->fGlass->GetName() : "-");

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
      ZMIR mir(M.mCbackAlpha, mCurrent);
      mi->ImprintMir(mir);
      mi->FixMirBits(mir, fImg->fEye->GetSaturnInfo());
      fImg->fEye->Send(mir);

      mCurrent = 0;
      Redraw(rd);
    }
    return 1;
  }

  return 0;
}
