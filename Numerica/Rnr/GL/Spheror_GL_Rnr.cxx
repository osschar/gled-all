// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Spheror_GL_Rnr.h"
#include <Rnr/GL/SphereTrings.h>
#include <FL/gl.h>

/**************************************************************************/

void Spheror_GL_Rnr::Draw(RnrDriver* rd)
{
  if(mSpheror->mNV == 0) return;

  TVectorF* v = mSpheror->GimmeXYZ(mSpheror->mState);
  glPushMatrix(); // Scaling!
  glScalef(mSpheror->mScale, mSpheror->mScale, mSpheror->mScale);
  glColor4fv(mSpheror->mColor());
  if(mSpheror->bBeautyP) {
    glEnable(GL_NORMALIZE);
  } else{
    glPointSize(mSpheror->mSize);
    glBegin(GL_POINTS);
  }
  Int_t i=0;
  for(Int_t f=0; f<mSpheror->mNV; f++, i+=3) {
    //cout << "Drawing " << (*v)(i) <<" "<< (*v)(i+1u) <<" "<< (*v)(i+2u) <<endl;
    if(mSpheror->bBeautyP) {
      glPushMatrix();
      glTranslatef((*v)(i),(*v)(i+1u),(*v)(i+2u));
      glScalef(mSpheror->mSize, mSpheror->mSize, mSpheror->mSize);
      SphereTrings::EnableGL(2);
      SphereTrings::DrawAndDisableGL(2);
      glPopMatrix();
    } else {
      glVertex3f((*v)(i),(*v)(i+1u),(*v)(i+2u));
      glNormal3f((*v)(i),(*v)(i+1u),(*v)(i+2u));
    }
  }
  delete v;
  if(mSpheror->bBeautyP) {
    glDisable(GL_NORMALIZE);
  } else {
    glEnd();
  }
  glPopMatrix();
}
