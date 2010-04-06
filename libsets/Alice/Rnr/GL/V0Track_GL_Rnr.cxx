// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "V0Track_GL_Rnr.h"
#include "HelixLineRnr.h"
#include <Glasses/RecTrackRS.h>

#include <TMath.h>

#include <GL/glew.h>

/**************************************************************************/

void V0Track_GL_Rnr::_init()
{}

/**************************************************************************/
/*
void V0Track_GL_Rnr::PreDraw(RnrDriver* rd)
{}

void V0Track_GL_Rnr::Draw(RnrDriver* rd)
{}

void V0Track_GL_Rnr::PostDraw(RnrDriver* rd)
{}
*/


void V0Track_GL_Rnr::Render(RnrDriver* rd)
{
  RNRDRIVER_GET_RNRMOD(srm, rd, RecTrackRS);
  RecTrackRS* rst_lens  = (RecTrackRS*) srm->fLens;
  ESDParticle* p = mV0Track->mESD;
  Float_t vx = p->fV[0], vy = p->fV[1], vz = p->fV[2];
  Float_t px = p->fP[0], py = p->fP[1], pz = p->fP[2];

  if (p->P() < rst_lens->mMinP) return;
  if(TMath::RadToDeg()*p->Theta() < (rst_lens->mTheta - rst_lens->mThetaOff) || 
     TMath::RadToDeg()*p->Theta() > (rst_lens->mTheta + rst_lens->mThetaOff)  )return;
  // check boundaries
  if(TMath::Abs(vz)>rst_lens->mMaxZ || (vx*vx + vy*vy) > (rst_lens->mMaxR)*(rst_lens->mMaxR)) return;

  // render particle as point
  glPointSize(rst_lens->mVertexSize);
  glBegin(GL_POINTS);
  glColor4fv(rst_lens->mVertexColor());
  glVertex3f(vx,vy,vz);
  glEnd();

  // show particle momentum
  if (rst_lens->mRnrP)
  {
    glLineWidth(2*rst_lens->mTrackWidth);
    glBegin(GL_LINES);
    glColor4fv(rst_lens->mPColor());
    glVertex3f(vx,vy,vz);
    Float_t ps = rst_lens->mPMinLen + TMath::Log10(1 + TMath::Sqrt(px*px+py*py+pz*pz)/rst_lens->mPScale);
    HPointF v(px,py,pz);
    v.Normalize(ps);
    glVertex3f(v.x + vx, v.y + vy, v.z + vz);
    glEnd();
  }

  // printf(" V0Track_GL_Rnr::Render \n");
  // propagate particle 
  if (rst_lens->mMaxR != 0)
  {
    glColor4fv(rst_lens->mTrackColor());
    //  printf("RecTrack_GL_Rnr draw with charg %d \n",p->fSign );
    glLineWidth(rst_lens->mTrackWidth);

    glBegin(GL_LINES);
    // from birth to decay 
    glVertex3f(vx, vy, vz);
    glVertex3f(p->fV[0],p->fV[1],p->fV[2]);
    // from decay to minus
    glVertex3f( p->fV[0],p->fV[1],p->fV[2]);
    glVertex3f(mV0Track->mVM[0],  mV0Track->mVM[1], mV0Track->mVM[2]);
    // from decay to plus
    glVertex3f( p->fV[0],p->fV[1],p->fV[2]);
    glVertex3f(mV0Track->mVP[0],  mV0Track->mVP[1], mV0Track->mVP[2]);
    glEnd();	

    // render point of DCA
    glPointSize(1.4*rst_lens->mVertexSize);
    glBegin(GL_POINTS);
    glColor4f(1.0,1.0, 1.0,1.0);
    glVertex3f(p->fV[0],p->fV[1],p->fV[2]);
    glEnd();

  }
}
