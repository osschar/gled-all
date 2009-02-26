// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGlMaterial
//
//

#include "ZGlMaterial.h"
#include "ZGlMaterial.c7"

#include <GL/glew.h>

ClassImp(ZGlMaterial);

/**************************************************************************/

void ZGlMaterial::_init()
{
  mMatOp     = O_On;
  mFace      = GL_FRONT_AND_BACK;
  mShininess = 64;
  mAmbient.rgba(0.2,0.2,0.2);
  mDiffuse.rgba(0.8,0.8,0.8);
  mSpecular.rgba(0,0,0);
  mEmission.rgba(0,0,0);

  mModeOp    = O_Nop;
  mModeFace  = GL_FRONT_AND_BACK;
  mModeColor = GL_AMBIENT_AND_DIFFUSE;
}

/**************************************************************************/


/**************************************************************************/
