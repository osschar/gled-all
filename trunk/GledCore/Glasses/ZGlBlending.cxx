// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGlBlending
//
//

#include "ZGlBlending.h"

#include "ZGlBlending.c7"

ClassImp(ZGlBlending)

/**************************************************************************/

void ZGlBlending::_init()
{
  // !!!! Set all links to 0 !!!!
  mBlendOp = GSO_Nop;
  mBSrcFac = GL_SRC_ALPHA;
  mBDstFac = GL_ONE_MINUS_SRC_ALPHA;

  mAntiAliasOp = GSO_Nop;
  bPointSmooth = true;
  mPointSize   = 1;
  mPointHint   = GL_NICEST;
  bLineSmooth  = true;
  mLineWidth   = 1;
  mLineHint    = GL_NICEST;

  mFogOp   = GSO_Nop;
  mFogMode = GL_EXP;
  mFogHint = GL_NICEST;
  mFogColor.rgba(0.1,0.1,0.1);
  mFogDensity = 0.2;
  mFogBeg     = 0;
  mFogEnd     = 20;
}

/**************************************************************************/


/**************************************************************************/
