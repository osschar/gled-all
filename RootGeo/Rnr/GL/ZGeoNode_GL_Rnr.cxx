// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGeoNode_GL_Rnr.h"
#include <FL/gl.h>

/**************************************************************************/

void ZGeoNode_GL_Rnr::_init()
{}

/**************************************************************************/


void ZGeoNode_GL_Rnr::Draw(RnrDriver* rd)
{
  ZGeoNode& N(*mZGeoNode);
  GeoUserData *ud = dynamic_cast<GeoUserData*>(N.GetVolumeField());
  if ( ud == 0 ) return;

  TGLFaceSet* fs = ud->fFaceSet;
  if(fs == 0) return;

  if (N.mColor[3] < 1) {
    glPushAttrib(GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glPushAttrib(GL_CURRENT_BIT);
  }
  
  glColor4fv(N.mColor());
  fs->GLDrawPolys();

  glPopAttrib();
}
