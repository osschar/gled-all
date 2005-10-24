// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Lamp_GL_Rnr.h"
#include <Rnr/GL/GLRnrDriver.h>
#include <Rnr/GL/SphereTrings.h>

#include <GL/gl.h>

//__________________________________________________________________________
// Lamp_GL_Rnr
//
//

/**************************************************************************/

void Lamp_GL_Rnr::Draw(RnrDriver* rd)
{
  if(mLampID == -1)
    if(mLamp->bOnIfOff)      LampOn(rd);
    else if(mLamp->bOffIfOn) LampOff(rd);
    
  if(mLampID != -1 && mLamp->bDrawLamp) {
    glPushMatrix();
    glPushAttrib(GL_CURRENT_BIT | GL_TRANSFORM_BIT);
    glEnable(GL_NORMALIZE);
    glColor4fv(mLamp->mDiffuse());
    glScalef(1, 0.5, 0.5);
    SphereTrings::EnableGL(0);
    SphereTrings::DrawAndDisableGL(0);
    glPopAttrib();
    glPopMatrix();
  }
}

void Lamp_GL_Rnr::CleanUp(RnrDriver* rd)
{
  LampOff(rd);
}

/**************************************************************************/

int Lamp_GL_Rnr::LampOn(RnrDriver* rd, const ZTrans* t)
{
  if(mLampID != -1) {
    return 1;
  }

  mLampID = rd->GL()->GetLamp(this);
  if(mLampID == -1) {
    cout <<"Lamp_GL_Rnr::LampOn No more available lights.\n";
    return 1;
  }

  if(t == 0) t = mLamp->PtrTrans();

  GLfloat x[4], d[4];
  x[0]=(*t)(1,4); x[1]=(*t)(2,4); x[2]=(*t)(3,4); x[3]=mLamp->mLampScale;
  d[0]=(*t)(1,1); d[1]=(*t)(2,1); d[2]=(*t)(3,1); d[3]=1;

  //printf("Lamp_GL_Rnr::LampOn turning on lamp id=%d (%.3f,%.3f,%.3f;%.3f)\n",
  //  mLampID, x[0], x[1], x[2], x[3]);

  GLenum L = (GLenum)(GL_LIGHT0 + mLampID);

  glLightfv(L, GL_POSITION, x);
  glLightfv(L, GL_DIFFUSE, mLamp->mDiffuse());
  glLightfv(L, GL_AMBIENT, mLamp->mAmbient());
  glLightfv(L, GL_SPECULAR, mLamp->mSpecular());

  if(mLamp->mLampScale > 0) {
    glLightfv(L, GL_SPOT_DIRECTION, d);
  }
  if(mLamp->mLampScale > 0 && mLamp->mSpotCutOff <= 90) {
    glLightf (L, GL_SPOT_CUTOFF, mLamp->mSpotCutOff);
    glLightf (L, GL_SPOT_EXPONENT, mLamp->mSpotExp);
  } else {
    glLightf (L, GL_SPOT_CUTOFF, 180);
  }

  glLightf(L, GL_CONSTANT_ATTENUATION, mLamp->mConstAtt);
  glLightf(L, GL_LINEAR_ATTENUATION, mLamp->mLinAtt);
  glLightf(L, GL_QUADRATIC_ATTENUATION, mLamp->mQuadAtt);

  glEnable(L);
  return 0;
}

int Lamp_GL_Rnr::LampOff(RnrDriver* rd)
{
  if(mLampID == -1) {
    return 1;
  }

  // cout <<"Lamp_GL_Rnr::LampOn turning off lamp id="<< mLampID <<endl;
  glDisable((GLenum)(GL_LIGHT0 + mLampID));
  rd->GL()->ReturnLamp(mLampID);
  mLampID = -1;
  return 0;
}
