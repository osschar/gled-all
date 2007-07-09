// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGlLightModel
//
//

#include "ZGlLightModel.h"

#include "ZGlLightModel.c7"

ClassImp(ZGlLightModel)

/**************************************************************************/

void ZGlLightModel::_init()
{
  mLightModelOp   = GSO_Nop;
  mLiMoAmbient.rgba(0.2, 0.2, 0.2);
  mLiMoColorCtrl = GL_SINGLE_COLOR;
  bLiMoLocViewer = false;
  bLiMoTwoSide   = false;

  mShadeModelOp = GSO_Nop;
  mShadeModel   = GL_SMOOTH;
  mFrontFace    = GL_CCW;
  mFrontMode    = GL_FILL;
  mBackMode     = GL_FILL;

  mFaceCullOp   = GSO_Nop;
  mFaceCullMode = GL_BACK;
}

/**************************************************************************/