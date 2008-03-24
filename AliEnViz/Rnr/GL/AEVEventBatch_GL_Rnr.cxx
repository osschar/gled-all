// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AEVEventBatch_GL_Rnr.h"
#include <Rnr/GL/SEvTaskState_GL_Rnr.h>
#include <Rnr/GL/GLTextNS.h>
#include <RnrBase/RnrDriver.h>
#include <Glasses/ZRlNodeMarkup.h>

#include <TMath.h>

#include <FL/gl.h>

/**************************************************************************/

void AEVEventBatch_GL_Rnr::_init()
{}

/**************************************************************************/

void AEVEventBatch_GL_Rnr::Draw(RnrDriver* rd)
{
  AEVEventBatch& EB = *mAEVEventBatch;  
  ZColor cols[4] = { EB.mColAll, EB.mColOK,  EB.mColFail,  EB.mColProc };

  glPushMatrix();
  glPushAttrib(GL_TRANSFORM_BIT);
  glEnable(GL_NORMALIZE);
  Float_t zscale = TMath::Max(TMath::Log10(EB.mEvState.GetNAll()), 1e-2);
  glScalef(1, EB.mHDepth, zscale);
  switch(EB.mRnrMode) {
  case AEVEventBatch::RM_Combined:
    SEvTaskState_GL_Rnr::RenderHisto(EB.mEvState, cols);
    glTranslatef(-2, 0, 0);
    SEvTaskState_GL_Rnr::RenderBar(EB.mEvState, cols);
    glTranslatef( 2, 0, 0);
    break;
  case AEVEventBatch::RM_Bar:
    glTranslatef(-0.5, -EB.mHDepth/2, 0);
    SEvTaskState_GL_Rnr::RenderBar(EB.mEvState, cols);
    glTranslatef(0.5, 0, 0);
    break;
  }
  glPopAttrib();
  glPopMatrix();


  glPushMatrix();
  string test(GForm("%s\n%d / %d", EB.GetName(), EB.mEvState.GetNOK(),
		    EB.mEvState.GetNAll()));
  glTranslatef(0, -0.6, 0);

  RNRDRIVER_GET_RNRMOD_LENS(nrc, rd, ZRlNodeMarkup);
  GLTextNS::BoxSpecs bs;  bs.pos = ""; bs.align = 'c';
  GLTextNS::RnrTextBar(rd, test, bs, nrc_lens->GetNameOffset());

  //glTranslatef(-0.5, -0.6, 0);
  //GLTextNS::RnrTextPoly(rd, test);
  glPopMatrix();

}
