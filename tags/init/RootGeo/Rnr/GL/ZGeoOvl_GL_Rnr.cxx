// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZGeoOvl_GL_Rnr.h"
#include <FL/gl.h>

/**************************************************************************/

void ZGeoOvl_GL_Rnr::_init()
{}

/**************************************************************************/


void ZGeoOvl_GL_Rnr::Draw(RnrDriver* rd)
{ 
  if (mZGeoOvl->GetRnrSelf()) {
    ZGeoNode_GL_Rnr::Draw(rd);
  }

  if(mZGeoOvl->mRnrMark == false ) return;
  
  glPopMatrix();
  Float_t* p = mZGeoOvl->mPM_p;
  if(p) {
    Int_t N = mZGeoOvl->mPM_N;
    glColor4fv(mZGeoOvl->mPM_Col());
    glBegin(GL_POINTS);
    for(int i = 0; i<N; ++i) {
      glVertex3fv(p);
      p += 3;
    }
    glEnd();
  }
  glPushMatrix();
}
