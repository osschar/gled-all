// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TPCSegFrame_GL_Rnr.h"
#include <RnrBase/Fl_Event_Enums.h>
#include <GL/gl.h>
#include <Rnr/GL/GLRnrDriver.h>

/**************************************************************************/

void TPCSegFrame_GL_Rnr::_init()
{
  mCurrent = 0;
}

/**************************************************************************/
void TPCSegFrame_GL_Rnr::Render(RnrDriver* rd)
{
  Float_t x,y,pl,hpw;
  glColor4f(1.,0.,0.,0.2);

  AliTPCParam* par = mTPCSegFrame->mTPCPar; 

  pl  = par->GetInnerPadPitchLength();
  hpw = par->GetInnerPadPitchWidth()/2.;
  for(int r = 0; r < par->GetNRowLow() ;r++){
    x = par->GetNPadsLow(r)*hpw;
    y = par->GetPadRowRadiiLow(r); 
    rd->GL()->PushName(this, (void*)r);
    glBegin(GL_QUADS);
    if (r == mCurrent) glColor4fv(mTPCSegFrame->mFocusCol());
    else glColor4fv(mTPCSegFrame->mRowCol());
    glVertex3f(-x, y,0.);
    glVertex3f(-x, y+pl,0.);
    glVertex3f( x, y+pl,0.);
    glVertex3f( x, y   ,0.);
    glEnd();
    rd->GL()->PopName();
  }

  Int_t nsr;
  nsr =  par->GetNRowLow();
  pl =  par->GetOuter1PadPitchLength();
  hpw = par->GetOuterPadPitchWidth()/2.;
  for(int r = 0; r < par->GetNRowUp1() ;r++){
    x = par->GetNPadsUp(r)*hpw;
    y = par->GetPadRowRadiiUp(r);
    rd->GL()->PushName(this, (void*)nsr);
    glBegin(GL_QUADS);
    if (nsr == mCurrent) glColor4fv(mTPCSegFrame->mFocusCol());
    else glColor4fv(mTPCSegFrame->mRowCol());
    glVertex3f(-x, y,0.);
    glVertex3f(-x, y+pl,0.);
    glVertex3f( x, y+pl,0.);
    glVertex3f( x, y   ,0.);
    glEnd();
    rd->GL()->PopName();
    nsr++;
  }

  pl =  par->GetOuter2PadPitchLength();
  for(int r = par->GetNRowUp1(); r< par->GetNRowUp() ;r++){
    x = par->GetNPadsUp(r)*hpw;
    y = par->GetPadRowRadiiUp(r);
    rd->GL()->PushName(this, (void*)nsr);
    glBegin(GL_QUADS);
    if (nsr == mCurrent) glColor4fv(mTPCSegFrame->mFocusCol());
    else glColor4fv(mTPCSegFrame->mRowCol());
    glVertex3f(-x, y,0.);
    glVertex3f(-x, y+pl,0.);
    glVertex3f( x, y+pl,0.);
    glVertex3f( x, y   ,0.);
    glEnd();
    rd->GL()->PopName();
    nsr++;
  }
}

/**************************************************************************/


int TPCSegFrame_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  if(ev.fEvent == FL_PUSH || ev.fEvent == FL_DRAG || ev.fEvent == FL_RELEASE) {
    // printf("TPCSegFrame_GL_Rnr::Handle picked %d \n", mCurrent);
    Int_t tgt = (int) ev.fCurrentNSE->fUserData;
    if(tgt != mCurrent) {
      // printf("enter new step %d in TPCSegFrame_GL_Rnr::Handle \n", tgt);
      mCurrent = tgt;
      Redraw(rd);      
      if(mTPCSegFrame->mPadRow)
	mTPCSegFrame->mPadRow->SetRow(mCurrent);
    }
    return 1;
  }
  return 0;
}
