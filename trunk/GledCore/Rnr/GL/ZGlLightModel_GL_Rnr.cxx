// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlLightModel_GL_Rnr.h"
#include <FL/gl.h>

/**************************************************************************/

void ZGlLightModel_GL_Rnr::_init()
{}

/**************************************************************************/

void ZGlLightModel_GL_Rnr::PreDraw(RnrDriver* rd)
{

}

void ZGlLightModel_GL_Rnr::Draw(RnrDriver* rd)
{
  ZGlLightModel& x = *mZGlLightModel;

  switch(x.mLigtModelOp) {
  case ZGlStateBase::GSO_On:
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, x.mLiMoAmbient());
    glLightModeli (GL_LIGHT_MODEL_COLOR_CONTROL, x.mLiMoColorCtrl);
    glLightModeli (GL_LIGHT_MODEL_LOCAL_VIEWER, x.bLiMoLocViewer);
    glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, x.bLiMoTwoSide);
    break;
  case ZGlStateBase::GSO_Off:
  case ZGlStateBase::GSO_Nop:
  default:
    break;
  }

  switch(x.mShadeModelOp) {
  case ZGlStateBase::GSO_On:
    glShadeModel(x.mShadeModel);
    glFrontFace(x.mFrontFace);
    glPolygonMode(GL_FRONT, x.mFrontMode);
    glPolygonMode(GL_BACK,  x.mBackMode);
    break;
  case ZGlStateBase::GSO_Off:
  case ZGlStateBase::GSO_Nop:
  default:
    break;
  }

  switch(x.mFaceCullOp) {
  case ZGlStateBase::GSO_On:
    glCullFace(x.mFaceCullMode);
    glEnable(GL_CULL_FACE);
    break;
  case ZGlStateBase::GSO_Off:
    glDisable(GL_CULL_FACE);
    break;
  case ZGlStateBase::GSO_Nop:
  default:
    break;
  }
}

void ZGlLightModel_GL_Rnr::PostDraw(RnrDriver* rd)
{

}
