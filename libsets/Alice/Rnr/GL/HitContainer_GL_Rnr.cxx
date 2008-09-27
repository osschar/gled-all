// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "HitContainer_GL_Rnr.h"
#include <FL/gl.h>

/**************************************************************************/

void HitContainer_GL_Rnr::_init()
{}

/**************************************************************************/

void HitContainer_GL_Rnr::Draw(RnrDriver* rd)
{
  HitContainer&	HC = *mHitContainer;
  if(HC.mNPoints > 0) {

    if(HC.mSize > 0) glPointSize(HC.mSize);
    glColor4fv(mHitContainer->mColor());

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glVertexPointer(3, GL_FLOAT, 0, HC.mPoints);
    glEnableClientState(GL_VERTEX_ARRAY);

    glDrawArrays(GL_POINTS, 0, HC.mNPoints);
    // In selection mode should loop over points and do push/pop name.

    glPopClientAttrib();
  }
}
