// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGlMaterial_GL_Rnr.h"
#include <FL/gl.h>

/**************************************************************************/

void ZGlMaterial_GL_Rnr::_init()
{}

/**************************************************************************/

void ZGlMaterial_GL_Rnr::PreDraw(RnrDriver* rd)
{
  // Ignore ZGlass::PreDraw
}

void ZGlMaterial_GL_Rnr::Draw(RnrDriver* rd)
{
  ZGlMaterial& x = *mZGlMaterial;

  if(x.mMatOp == ZGlStateBase::GSO_On) {
    glMaterialf(x.mFace, GL_SHININESS, x.mShininess);
    glMaterialfv(x.mFace, GL_AMBIENT, x.mAmbient());
    glMaterialfv(x.mFace, GL_DIFFUSE, x.mDiffuse());
    glMaterialfv(x.mFace, GL_SPECULAR, x.mSpecular());
    glMaterialfv(x.mFace, GL_EMISSION, x.mEmission());
  }

  switch(x.mModeOp) {
  case ZGlStateBase::GSO_On:
    glColorMaterial(x.mModeFace, x.mModeColor);
    glEnable(GL_COLOR_MATERIAL);
    break;
  case ZGlStateBase::GSO_Off:
    glDisable(GL_COLOR_MATERIAL);
    break;
  case ZGlStateBase::GSO_Nop:
  default:
    break;
  }
}

void ZGlMaterial_GL_Rnr::PostDraw(RnrDriver* rd)
{
  // Ignore ZGlass::PostDraw
}
