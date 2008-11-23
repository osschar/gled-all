// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGlBlending
//
//

#include "ZGlBlending.h"

#include "ZGlBlending.c7"

ClassImp(ZGlBlending);

/**************************************************************************/

void ZGlBlending::_init()
{
  mBlendOp   = O_Nop;
  mBSrcFac   = GL_SRC_ALPHA;
  mBDstFac   = GL_ONE_MINUS_SRC_ALPHA;
  mBEquation = GL_FUNC_ADD;
  mBConstCol.rgba(0.5, 0.5, 0.5, 0.5);

  mAntiAliasOp = O_Nop;
  bPointSmooth = true;
  mPointSize   = 1;
  mPointHint   = GL_FASTEST;
  bLineSmooth  = true;
  mLineWidth   = 1;
  mLineHint    = GL_FASTEST;

  mFogOp   = O_Nop;
  mFogMode = GL_EXP;
  mFogHint = GL_FASTEST;
  mFogColor.rgba(0.1,0.1,0.1);
  mFogDensity = 0.2;
  mFogBeg     = 0;
  mFogEnd     = 20;

  mDepthOp   = O_Nop;
  mDepthFunc = GL_LESS;
  mDepthMaskOp = O_Nop;
}

/**************************************************************************/


/**************************************************************************/
