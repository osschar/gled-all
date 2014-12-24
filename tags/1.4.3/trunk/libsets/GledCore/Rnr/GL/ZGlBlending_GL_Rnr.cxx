// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlBlending_GL_Rnr.h"
#include "GLRnrDriver.h"
#include <GL/glew.h>

#define PARENT ZRnrModBase_GL_Rnr

/**************************************************************************/

void ZGlBlending_GL_Rnr::_init()
{}

/**************************************************************************/

void ZGlBlending_GL_Rnr::PreDraw(RnrDriver* rd)
{
  PARENT::PreDraw(rd);
  glPushAttrib(GL_COLOR_BUFFER_BIT | GL_POINT_BIT |
               GL_LINE_BIT         | GL_FOG_BIT   |
	       GL_DEPTH_BUFFER_BIT);
  SetupGL(rd);
}

void ZGlBlending_GL_Rnr::Draw(RnrDriver* rd)
{
  SetupGL(rd);
}

void ZGlBlending_GL_Rnr::PostDraw(RnrDriver* rd)
{
  glPopAttrib();
  PARENT::PostDraw(rd);
}

/**************************************************************************/

void ZGlBlending_GL_Rnr::SetupGL(RnrDriver* rd)
{
  ZGlBlending& x = *mZGlBlending;

  switch(x.mBlendOp) {
  case ZRnrModBase::O_On:
    glBlendFunc(x.mBSrcFac, x.mBDstFac);
    glBlendEquation(x.mBEquation);
    glBlendColor(x.mBConstCol.r(), x.mBConstCol.g(), x.mBConstCol.b(), x.mBConstCol.a());
    glEnable(GL_BLEND);
    break;
  case ZRnrModBase::O_Off:
    glDisable(GL_BLEND);
    break;
  case ZRnrModBase::O_Nop:
  default:
    break;
  }

  switch(x.mAntiAliasOp) {
  case ZRnrModBase::O_On:
    rd->GL()->PointSize(x.mPointSize);
    glHint(GL_POINT_SMOOTH_HINT, x.mPointHint);
    if(x.bPointSmooth)
      glEnable(GL_POINT_SMOOTH);
    else
      glDisable(GL_POINT_SMOOTH);

    rd->GL()->LineWidth(x.mLineWidth);
    glHint(GL_LINE_SMOOTH_HINT, x.mLineHint);
    if(x.bLineSmooth)
      glEnable(GL_LINE_SMOOTH);
    else
      glDisable(GL_LINE_SMOOTH);
    break;
  case ZRnrModBase::O_Off:
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
    break;
  case ZRnrModBase::O_Nop:
  default:
    break;
  }

  switch(x.mFogOp) {
  case ZRnrModBase::O_On:
    glFogi(GL_FOG_MODE, x.mFogMode);
    glHint(GL_FOG_HINT, x.mFogHint);
    glFogfv(GL_FOG_COLOR, x.mFogColor());
    glFogf(GL_FOG_DENSITY, x.mFogDensity);
    glFogf(GL_FOG_START, x.mFogBeg);
    glFogf(GL_FOG_END, x.mFogEnd);
    glEnable(GL_FOG);
    break;
  case ZRnrModBase::O_Off:
    glDisable(GL_FOG);
    break;
  case ZRnrModBase::O_Nop:
  default:
    break;
  }

  switch(x.mDepthOp) {
  case ZRnrModBase::O_On:
    glDepthFunc(x.mDepthFunc);
    glEnable(GL_DEPTH_TEST);
    break;
  case ZRnrModBase::O_Off:
    glDisable(GL_DEPTH_TEST);
    break;
  case ZRnrModBase::O_Nop:
  default:
    break;
  }
  switch(x.mDepthMaskOp) {
  case ZRnrModBase::O_On:
    glDepthMask(1);
    break;
  case ZRnrModBase::O_Off:
    glDepthMask(0);
    break;
  case ZRnrModBase::O_Nop:
  default:
    break;
  }

}
