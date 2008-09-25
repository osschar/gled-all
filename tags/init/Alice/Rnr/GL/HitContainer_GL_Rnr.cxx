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
  const HitContainer::vHit_t& hits = mHitContainer->RefHits();
  glBegin(GL_POINTS);
  if(! hits.empty()) {  
    glColor4fv(mHitContainer->mColor());
    for(HitContainer::vHit_ci i=hits.begin(); i!=hits.end(); ++i) {
      const Hit* h = (*i);
      glVertex3f(h->x, h->y, h->z);
    }
  }
  glEnd();

}
