// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlBlending_GL_Rnr.h"
#include <FL/gl.h>

/**************************************************************************/

void ZGlBlending_GL_Rnr::_init()
{}

/**************************************************************************/

void ZGlBlending_GL_Rnr::PreDraw(RnrDriver* rd)
{
  // Ignore ZGlass::PreDraw
}

void ZGlBlending_GL_Rnr::Draw(RnrDriver* rd)
{
  ZGlBlending& x = *mZGlBlending;

  switch(x.mBlendOp) {
  case ZGlStateBase::GSO_On:
    glBlendFunc(x.mBSrcFac, x.mBDstFac);
    glEnable(GL_BLEND);
    break;
  case ZGlStateBase::GSO_Off:
    glDisable(GL_BLEND);
    break;
  case ZGlStateBase::GSO_Nop:
  default:
    break;
  }

  switch(x.mAntiAliasOp) {
  case ZGlStateBase::GSO_On:
    glPointSize(x.mPointSize);
    glHint(GL_POINT_SMOOTH_HINT, x.mPointHint);
    if(x.bPointSmooth)
      glEnable(GL_POINT_SMOOTH);
    else
      glDisable(GL_POINT_SMOOTH);

    glLineWidth(x.mLineWidth);
    glHint(GL_LINE_SMOOTH_HINT, x.mLineHint);
    if(x.bLineSmooth)
      glEnable(GL_LINE_SMOOTH);
    else
      glDisable(GL_LINE_SMOOTH);
    break;
  case ZGlStateBase::GSO_Off:
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
    break;
  case ZGlStateBase::GSO_Nop:
  default:
    break;
  }

  switch(x.mFogOp) {
  case ZGlStateBase::GSO_On:
    glFogi(GL_FOG_MODE, x.mFogMode);
    glHint(GL_FOG_HINT, x.mFogHint);
    glFogfv(GL_FOG_COLOR, x.mFogColor());
    glFogf(GL_FOG_DENSITY, x.mFogDensity);
    glFogf(GL_FOG_START, x.mFogBeg);
    glFogf(GL_FOG_END, x.mFogEnd);
    glEnable(GL_FOG);
    break;
  case ZGlStateBase::GSO_Off:
    glDisable(GL_FOG);
    break;
  case ZGlStateBase::GSO_Nop:
  default:
    break;
  }

}

void ZGlBlending_GL_Rnr::PostDraw(RnrDriver* rd)
{
  // Ignore ZGlass::PostDraw
}
