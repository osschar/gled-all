// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGeoOvl_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <GL/gl.h>

/**************************************************************************/

void ZGeoOvl_GL_Rnr::_init()
{}

/**************************************************************************/


void ZGeoOvl_GL_Rnr::Draw(RnrDriver* rd)
{
  if (mZGeoOvl->mRnrNode) {
    ZGeoNode_GL_Rnr::Draw(rd);
  }

  Float_t* p = mZGeoOvl->mPM_p;
  if (mZGeoOvl->mRnrMark && p != 0) {
    Int_t N = mZGeoOvl->mPM_N;
    rd->GL()->Color(mZGeoOvl->mPM_Col);
    glBegin(GL_POINTS);
    for(int i = 0; i<N; ++i) {
      glVertex3fv(p);
      p += 3;
    }
    glEnd();
  }
}
