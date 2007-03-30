// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Lamp_GL_Rnr.h"
#include <RnrBase/RnrDriver.h>
//#include <Stones/ZTrans.h>

#include <FL/gl.h>

//__________________________________________________________________________
// Lamp_GL_Rnr
//
//

/**************************************************************************/

void Lamp_GL_Rnr::Draw(RnrDriver* rd)
{
  if(mLampID == -1)
    if(mLamp->bOnIfOff) LampOn(rd);
  else
    if(mLamp->bOffIfOn) LampOff(rd);
    
  if(mLampID != -1 && mLamp->bDrawLamp) {
    // Here should render lamp as geometric object.
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
    cout <<"Lamp_GL_Rnr::LampOn Lamp seems to be on ...\n";
    return 1;
  }

  mLampID = rd->GetLamp(this);
  if(mLampID == -1) {
    cout <<"Lamp_GL_Rnr::LampOn No available lights ...\n";
    return 1;
  }

  // cout <<"Lamp_GL_Rnr::LampOn turning on lamp id="<< mLampID <<endl;

  if(t == 0) t = mLamp->PtrTrans();

  GLfloat x[4], d[4];
  // !!! should lock master
  x[0]=(*t)(1u,4u); x[1]=(*t)(2u,4u); x[2]=(*t)(3u,4u); x[3]= mLamp->mScale;
  d[0]=(*t)(1u,1u); d[1]=(*t)(2u,1u); d[2]=(*t)(3u,1u); d[3]=1;

  GLenum L = (GLenum)(GL_LIGHT0 + mLampID);

  glLightfv(L, GL_POSITION, x);
  glLightfv(L, GL_DIFFUSE, mLamp->mDiffuse());
  glLightfv(L, GL_AMBIENT, mLamp->mAmbient());
  glLightfv(L, GL_SPECULAR, mLamp->mSpecular());

  if(mLamp->mScale > 0) {
    glLightfv(L, GL_SPOT_DIRECTION, d);
  }
  if(mLamp->mScale > 0 && mLamp->mSpotCutOff <= 90) {
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
    cout <<"Lamp_GL_Rnr::LampOff Lamp doesn't seem to be on ...\n";
    return 1;
  }

  // cout <<"Lamp_GL_Rnr::LampOn turning off lamp id="<< mLampID <<endl;
  glDisable((GLenum)(GL_LIGHT0 + mLampID));
  rd->ReturnLamp(mLampID);
  mLampID = -1;
  return 0;
}