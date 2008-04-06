// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SMorph_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/TubeTvor_GL_Rnr.h>

/**************************************************************************/

void SMorph_GL_Rnr::Render(RnrDriver* rd)
{
  glPushAttrib(GL_CURRENT_BIT);
  rd->GL()->Color(mSMorph->mColor);
  if(mSMorph->pTuber) TubeTvor_GL_Rnr::Render(mSMorph->pTuber);
  glPopAttrib();
}

void SMorph_GL_Rnr::Triangulate(RnrDriver* rd)
{
  ZNode_GL_Rnr::Triangulate(rd);
  mSMorph->Triangulate();
}

/**************************************************************************/
