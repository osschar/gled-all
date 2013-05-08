// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGeoNode_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>

#include <TGLFaceSet.h>
#include <TGLRnrCtx.h>

#include <GL/glew.h>

/**************************************************************************/

void ZGeoNode_GL_Rnr::_init()
{}

/**************************************************************************/

namespace {
TGLRnrCtx sgRnrCtx(0);
}

void ZGeoNode_GL_Rnr::Draw(RnrDriver* rd)
{
  ZGeoNode& N(*mZGeoNode);
  GeoUserData *ud = dynamic_cast<GeoUserData*>(N.GetVolumeField());
  if ( ud == 0 ) return;

  TGLFaceSet* fs = ud->fFaceSet;
  if(fs == 0) return;

  Float_t alpha = rd->GL()->Color(N.mColor);
  if (alpha < 1) {
    glPushAttrib(GL_CURRENT_BIT | GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glPushAttrib(GL_CURRENT_BIT);
  }

  fs->Draw(sgRnrCtx);

  glPopAttrib();
}
