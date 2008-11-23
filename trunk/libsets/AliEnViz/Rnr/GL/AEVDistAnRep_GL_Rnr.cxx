// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AEVDistAnRep_GL_Rnr.h"
#include <Rnr/GL/SEvTaskState_GL_Rnr.h>
#include <Stones/ZColor.h>
#include <Rnr/GL/GLTextNS.h>
#include <RnrBase/RnrDriver.h>
#include <Glasses/ZRlNodeMarkup.h>

#include <FL/gl.h>

namespace {
  ZColor batch_cols[4] = {
    ZColor( 0.1, 0.1, 0.8 ),
    ZColor( 0, 0.8, 0.2 ),
    ZColor( 0.8, 0.1, 0.4 ),
    ZColor( 0.8, 0.78, 0.26 )
  };
}

/**************************************************************************/

void AEVDistAnRep_GL_Rnr::_init()
{}

/**************************************************************************/

//void AEVDistAnRep_GL_Rnr::PreDraw(RnrDriver* rd)
//{}

void AEVDistAnRep_GL_Rnr::Draw(RnrDriver* rd)
{
  ZColor cols[4];
  cols[0].rgba (0.1, 0.1, 0.8);
  cols[1].rgba  (0,   0.8, 0.2);
  cols[2].rgba(0.8,   0, 0.2);
  cols[3].rgba(0.1, 0.6, 0.6);

  AEVDistAnRep& DAR = *mAEVDistAnRep;

  RNRDRIVER_GET_RNRMOD_LENS(nrc, rd, ZRlNodeMarkup);
  Float_t txtoff = nrc_lens->GetNameOffset();
  GLTextNS::BoxSpecs bs;  bs.pos = "b"; bs.align = 'c';

  glPushMatrix();
  glTranslatef(-1.1, -0.5, 0);
  SEvTaskState_GL_Rnr::RenderBar(DAR.mBatchState, batch_cols);
  glTranslatef(0.5, 0, 0);
  {
    bs.pos = "br";
    glPushMatrix();
    string test(GForm("Sites\n%d / %d", DAR.mBatchState.GetNOK(), DAR.mBatchState.GetNAll()));
    glTranslatef(0, -0.2, 0);
    GLTextNS::RnrTextBar(rd, test, bs, txtoff);
    glPopMatrix();
  }

  glTranslatef(0.7, 0, 0);
  SEvTaskState_GL_Rnr::RenderBar(DAR.mEventState, 0);
  glTranslatef(0.5, 0, 0);
  {
    bs.pos = "bl";
    glPushMatrix();
    string test(GForm("Events\n%d / %d", DAR.mEventState.GetNOK(), DAR.mEventState.GetNAll()));
    glTranslatef(0, -0.2, 0);
    GLTextNS::RnrTextBar(rd, test, bs, txtoff);
    glPopMatrix();
  }
  glPopMatrix();
}

//void AEVDistAnRep_GL_Rnr::PostDraw(RnrDriver* rd)
//{}
