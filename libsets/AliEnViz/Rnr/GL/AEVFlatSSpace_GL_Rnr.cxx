// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AEVFlatSSpace_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
#include <FL/gl.h>

/**************************************************************************/

void AEVFlatSSpace_GL_Rnr::_init()
{}

/**************************************************************************/

void AEVFlatSSpace_GL_Rnr::PreDraw(RnrDriver* rd)
{
  Board_GL_Rnr::PreDraw(rd);
}

void AEVFlatSSpace_GL_Rnr::Draw(RnrDriver* rd)
{
  Board_GL_Rnr::Draw(rd);
}

void AEVFlatSSpace_GL_Rnr::PostDraw(RnrDriver* rd)
{
  Board_GL_Rnr::PostDraw(rd);
}
