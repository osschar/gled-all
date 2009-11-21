// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZGlPerspective
//
// Sets-up GL projection and modelview matrices for orthographic
// viewing in fixed or pixel-based coordinates.
//
// Negative offsets are handled specially in OrthoPixel mode - they are
// interpreted as distance from right / upper edge.
//
// Mode OrthoTrueAspect takes the smaller of actual w/h as a unit and
// creates a (-1,1) view with center at the screen center.
// For now mOrthoW/H and mOx/y/z are ignored.

#include "ZGlPerspective.h"
#include "ZGlPerspective.c7"

ClassImp(ZGlPerspective);

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

void ZGlPerspective::StandardTrueAspect()
{
  ZGlPerspective& X = *this;
  X.SetViewMode(VM_OrthoTrueAspect);
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
