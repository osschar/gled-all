// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGlPerspective
//
// Sets-up GL projection and modelview matrices for orthographic
// viewing in fixed or pixel-based coordinates.
//
// TODO:
// a) handling of negative offsets (need origin mode, or some flags)
// b) for fixed mode, set w/h in accordance with window size (optional)

#include "ZGlPerspective.h"
#include "ZGlPerspective.c7"

ClassImp(ZGlPerspective)

/**************************************************************************/

void ZGlPerspective::_init()
{
  mViewMode  = VM_Nop;
  mOrthoW    = 10; mOrthoH   = 10;
  mOrthoNear = -1; mOrthoFar = 1;
  mOx = mOy = mOz = 0;
}

/**************************************************************************/

void ZGlPerspective::StandardPersp()
{
  ZGlPerspective& X = *this;
  X.SetViewMode(VM_Nop);
  X.SetOrthoW(10.000000); X.SetOrthoH(10.000000);
  X.SetOrthoNear(-1.000000); X.SetOrthoFar(1.000000);
}

void ZGlPerspective::StandardFixed()
{
  ZGlPerspective& X = *this;
  X.SetViewMode(VM_OrthoFixed);
  X.SetOrthoW(10.000000); X.SetOrthoH(10.000000);
  X.SetOrthoNear(-1.000000); X.SetOrthoFar(1.000000);
}

void ZGlPerspective::StandardPixel()
{
  ZGlPerspective& X = *this;
  X.SetViewMode(VM_OrthoPixel);
  X.SetOrthoW(10.000000); X.SetOrthoH(10.000000);
  X.SetOrthoNear(-1.000000); X.SetOrthoFar(1.000000);
}

/**************************************************************************/
