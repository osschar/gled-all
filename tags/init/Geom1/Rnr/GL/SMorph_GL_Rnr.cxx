// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SMorph_GL_Rnr.h"
#include <Rnr/GL/TubeTvor_GL_Rnr.h>

#include <FL/gl.h>

void SMorph_GL_Rnr::Draw(RnrDriver* rd)
{
  glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT);

  glPushMatrix(); // Scaling!
  if(mSMorph->bNormalize) glEnable(GL_NORMALIZE);
  glColor4fv(mSMorph->mColor());
  glScalef(mSMorph->mSx, mSMorph->mSy, mSMorph->mSz);

  if(mSMorph->pTuber) TubeTvor_GL_Rnr::Render(mSMorph->pTuber);

  if(mSMorph->bNormalize) glDisable(GL_NORMALIZE);

  glPopMatrix();
  glPopAttrib();
}

void SMorph_GL_Rnr::Triangulate()
{
  ZNode_GL_Rnr::Triangulate();
  mSMorph->Triangulate();
}