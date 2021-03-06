// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "RecTrack_GL_Rnr.h"
#include "HelixLineRnr.h"
#include <Glasses/RecTrackRS.h>

#include <TMath.h>

#include <GL/glew.h>

/**************************************************************************/

void RecTrack_GL_Rnr::_init()
{
 mTrackRMS.fFid = RecTrackRS::FID();
}

/**************************************************************************/

void RecTrack_GL_Rnr::Draw(RnrDriver* rd)
{
  obtain_rnrmod(rd, mTrackRMS);
  ZNode_GL_Rnr::Draw(rd);
}

/**************************************************************************/

namespace {
  inline double dphi_deg(const double p1, const double p2) {
    double r = p1 - p2;
    if(r > 180)     r -= 360;
    else if(r < -180) r += 360;
    return r;
  }
}

void RecTrack_GL_Rnr::Render(RnrDriver* rd)
{
  ESDParticle* p = mRecTrack->mESD;
  if(p == 0) return;

  RecTrackRS* rst_lens  = (RecTrackRS*) mTrackRMS.lens();

  Float_t vx = p->fV[0], vy = p->fV[1], vz = p->fV[2];
  Float_t px = p->fP[0], py = p->fP[1], pz = p->fP[2];

  if (p->P() < rst_lens->mMinP) return;
  if(TMath::Abs(TMath::RadToDeg()*p->Theta() - rst_lens->mTheta) >
     rst_lens->mThetaOff)       return;
  if(TMath::Abs(dphi_deg(TMath::RadToDeg()*p->Phi(), rst_lens->mPhi)) >
     rst_lens->mPhiOff)         return;

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

  // propagate particle 
  if (rst_lens->mMaxR != 0 && p->fSign)
  {
    bool stipple_off_p = false;
    if(rst_lens->mTrackStippleFac) {
      stipple_off_p = (glIsEnabled(GL_LINE_STIPPLE) == false);
      glLineStipple(rst_lens->mTrackStippleFac, rst_lens->mTrackStipplePat);
      glEnable(GL_LINE_STIPPLE);
    }

    glColor4fv(rst_lens->mTrackColor());
    //  printf("RecTrack_GL_Rnr draw with charg %d \n",p->fSign );
    glLineWidth(rst_lens->mTrackWidth);

    /*
      A_Rnr* irnr = 0;
      if(rst_lens->mTexture) {
      irnr = rd->GetLensRnr(rst_lens->mTexture);
      irnr->PreDraw(rd);
      }
    */
    Float_t a = 0.2998*rst_lens->mMagField*p->fSign/100; // m->cm
    Helix helix(a, rst_lens);
    helix.init( TMath::Sqrt(px*px+py*py), pz);
    helix.loop_to_bounds(vx, vy, vz, px, py, pz);
    /*
      if(irnr) irnr->PostDraw(rd);
    */

    if(stipple_off_p) glDisable(GL_LINE_STIPPLE);
  }
}
