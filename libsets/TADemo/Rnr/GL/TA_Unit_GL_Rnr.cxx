// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TA_Unit_GL_Rnr.h"
#include <GL/glew.h>

/**************************************************************************/

void TA_Unit_GL_Rnr::PreDraw(RnrDriver* rd)
{
  TA_SubUnit_GL_Rnr::PreDraw(rd);
  glPushMatrix(); glScalef(mTA_Unit->mS, mTA_Unit->mS, mTA_Unit->mS);
  glColor4fv(mTA_Unit->mColor());
}

void TA_Unit_GL_Rnr::PostDraw(RnrDriver* rd)
{
  glPopMatrix();
  TA_SubUnit_GL_Rnr::PostDraw(rd);
}
