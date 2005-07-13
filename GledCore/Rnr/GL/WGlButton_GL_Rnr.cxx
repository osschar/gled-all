// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "WGlButton_GL_Rnr.h"

#include <RnrBase/Fl_Event_Enums.h>
#include <Rnr/GL/ZRlFont_GL_Rnr.h>
#include <Rnr/GL/WGlFrameStyle_GL_Rnr.h>

#include <Stones/ZMIR.h>
#include <Eye/Eye.h>

#include <GL/gl.h>

/**************************************************************************/

void WGlButton_GL_Rnr::_init()
{
  bBelowMouse = false;
}

// direct connect to bool data-member, for light button:
// dm=foo->Class()->GetDataMember("bAlpha")
// *(Bool_t*)((char* foo) + dm->GetOffset())


/**************************************************************************/

void WGlButton_GL_Rnr::Draw(RnrDriver* rd)
{
  obtain_rnrmod(rd, mFontRMS);
  obtain_rnrmod(rd, mFrameRMS);
  
  GLTextNS::TexFont    *txf = ((ZRlFont_GL_Rnr*)mFontRMS.rnr())->GetFont();
  WGlFrameStyle_GL_Rnr &FSR = *(WGlFrameStyle_GL_Rnr*)mFrameRMS.rnr();

  WGlButton& B = *mWGlButton;
  string label = B.GetName();
  FSR.FullRender(txf, label, B.mDx, B.mDy, bBelowMouse);
}

/**************************************************************************/

int WGlButton_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  WGlButton& B = *mWGlButton;

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

  if(ev.fEvent == FL_PUSH && ev.fButton == FL_LEFT_MOUSE) {
    GledNS::MethodInfo* mi = B.GetCbackMethodInfo();
    if(mi == 0) return 0;
    ZMIR mir(B.mCbackAlpha);
    mi->ImprintMir(mir);
    mi->FixMirBits(mir, fImg->fEye->GetSaturnInfo());
    fImg->fEye->Send(mir);

    bBelowMouse = false;
    Redraw(rd);
    return 1;
  }
  if(ev.fEvent == FL_RELEASE) {
    bBelowMouse = true;
    Redraw(rd);
    return 1;
  }

  return 0;
}
