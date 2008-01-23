// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "PSMark_GL_Rnr.h"
#include <GL/gl.h>

/**************************************************************************/

void PSMark_GL_Rnr::_init()
{}

/**************************************************************************/

void PSMark_GL_Rnr::Render(RnrDriver* rd)
{
  GL_Capability_Switch loff(GL_LIGHTING, false);

  // draw point / sphere ??
  glBegin(GL_LINES);
  glColor3f(1, 0, 0); glVertex3f(0, 0, 0); glVertex3f(1, 0, 0); 
  glColor3f(0, 1, 0); glVertex3f(0, 0, 0); glVertex3f(0, 1, 0); 
  glColor3f(0, 0, 1); glVertex3f(0, 0, 0); glVertex3f(0, 0, 1); 
  glEnd();
}
