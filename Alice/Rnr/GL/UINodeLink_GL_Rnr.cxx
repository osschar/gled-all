// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "UINodeLink_GL_Rnr.h"

#include <RnrBase/Fl_Event_Enums.h>
#include <Rnr/GL/ZRlFont_GL_Rnr.h>
#include <Rnr/GL/WGlFrameStyle_GL_Rnr.h>

#include <Stones/ZMIR.h>
#include <Eye/Eye.h>

#include <GL/gl.h>

/**************************************************************************/

void UINodeLink_GL_Rnr::_init()
{}

/**************************************************************************/

int UINodeLink_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  if(mUINodeLink->mActive == false) return 0;
  
  if(ev.fEvent == FL_LEAVE) {
    bBelowMouse = false;
    mUINodeLink->SetHasFocus(false);
    GledNS::MethodInfo* mi = mUINodeLink->GetFocusMethodInfo();
    if(mi == 0) return 0;
    ZMIR mir(*mUINodeLink->mFocusAlpha, *mUINodeLink->mFocusBeta);
    mi->ImprintMir(mir);
    fImg->fEye->Send(mir);
    Redraw(rd);
    return 1;
  }
  
  if(ev.fEvent == FL_ENTER) {
    bBelowMouse = true;
    mUINodeLink->SetHasFocus(true);
    GledNS::MethodInfo* mi = mUINodeLink->GetFocusMethodInfo();
    if(mi == 0) return 0;
    ZMIR mir(*mUINodeLink->mFocusAlpha, *mUINodeLink->mFocusBeta);
    mi->ImprintMir(mir);
    fImg->fEye->Send(mir);
    Redraw(rd);
    return 1;
  }

  if(ev.fEvent == FL_PUSH && ev.fButton == FL_LEFT_MOUSE) {
    GledNS::MethodInfo* mi = mUINodeLink->GetCbackMethodInfo();
    if(mi == 0) return 0;
    ZMIR mir(*mUINodeLink->mCbackAlpha, *mUINodeLink->mCbackBeta);
    mi->ImprintMir(mir);
    fImg->fEye->Send(mir);
    bBelowMouse = false;
    Redraw(rd);
    return 1;
  }
  if(ev.fEvent == FL_RELEASE) {
    bBelowMouse = true;
    return 1;
  }

  return 0;
}
