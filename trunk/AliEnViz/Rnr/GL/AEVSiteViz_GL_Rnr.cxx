// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AEVSiteViz_GL_Rnr.h"
#include <Rnr/GL/SphereTrings.h>
#include <FL/gl.h>

/**************************************************************************/

void AEVSiteViz_GL_Rnr::_init()
{}

/**************************************************************************/

void AEVSiteViz_GL_Rnr::Draw(RnrDriver* rd)
{
  AEVSiteViz& A = *mAEVSiteViz;

  glColor4fv(A.mColor());
  glPushMatrix();
  glScalef(0.5, 0.5, 0.5);
  SphereTrings::EnableGL(2);
  SphereTrings::DrawAndDisableGL(2);
  glPopMatrix();
}
