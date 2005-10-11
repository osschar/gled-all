// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "MCTrack_GL_Rnr.h"
#include <Glasses/MCTrackRnrStyle.h>
#include <FL/gl.h>

void MCTrack_GL_Rnr::_init()
{
  mParticleRMS.fFid = MCTrackRnrStyle::FID();

  mStampPointCalc = 0;
}

/**************************************************************************/

void MCTrack_GL_Rnr::PreDraw(RnrDriver* rd)
{
  // Completely ignore position information.
  ZGlass_GL_Rnr::PreDraw(rd);
}

void MCTrack_GL_Rnr::Draw(RnrDriver* rd)
{
  obtain_rnrmod(rd, mParticleRMS);
  ZNode_GL_Rnr::Draw(rd);
}

void MCTrack_GL_Rnr::PostDraw(RnrDriver* rd)
{
  ZGlass_GL_Rnr::PostDraw(rd);
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

void MCTrack_GL_Rnr::Render(RnrDriver* rd)
{
  MCParticle* p = mMCTrack->mParticle;
  Bool_t show_p = true;

  MCTrackRnrStyle* rst_lens  = (MCTrackRnrStyle*) mParticleRMS.lens();

  Float_t vx = p->Vx(), vy=p->Vy(), vz=p->Vz();
  Float_t px = p->Px(), py=p->Py(), pz=p->Pz();  
  Float_t t_scale  = TMath::Power(10, rst_lens->mMaxTScale);
  Float_t max_time = rst_lens->mMaxT * t_scale;
  Float_t min_time = rst_lens->mMinT * t_scale;

  if (p->P() < rst_lens->mMinP) show_p = false;
  if(TMath::Abs(TMath::RadToDeg()*p->Theta() - rst_lens->mTheta) >
     rst_lens->mThetaOff)       show_p = false;
  if(TMath::Abs(dphi_deg(TMath::RadToDeg()*p->Phi(), rst_lens->mPhi)) >
     rst_lens->mPhiOff)         show_p = false;

  
  if(rst_lens->mCheckT && p->T() > max_time)
    show_p = false;

  if(p->GetPDG() == 0) show_p = false;

  // check boundaries
  if(TMath::Abs(vz) > rst_lens->mMaxZ || (vx*vx + vy*vy) > (rst_lens->mMaxR)*(rst_lens->mMaxR)) 
    show_p = false;

  if(rst_lens->mForceVisParents && mMCTrack->bRnrElements != show_p) {
    mMCTrack->SetRnrElements(show_p);
  } 

  if(show_p == false)
    return;

  //render particle as point
  if(p->T() > min_time) {
    glPointSize(rst_lens->mVertexSize);
    glBegin(GL_POINTS);
    glColor4fv(rst_lens->mVertexColor());
    glVertex3f(vx,vy,vz);
    glEnd();
  }
  
  // show particle momentum
  if(rst_lens->mRnrP) {
    if(rst_lens->mTrackWidth) glLineWidth(2*rst_lens->mTrackWidth);
    glBegin(GL_LINES);
    glColor4fv(rst_lens->mPColor());
    glVertex3f(vx,vy,vz);
    Float_t ps = rst_lens->mPMinLen + TMath::Log10(1 + p->P()/rst_lens->mPScale);
    TVector3 v(px,py,pz);
    v = ps*v.Unit();
    glVertex3f(v.X() + vx, v.Y() + vy, v.Z() + vz);
    glEnd();
  }

  // calculate points and save them to vector
  if(mStampPointCalc < rst_lens->mStampPointCalcReq) {
    track_points.clear();
    make_track(rd);
    mStampPointCalc = rst_lens->mStampPointCalcReq;
  }

  if(rst_lens->mCheckT)
    vertices_foo = &MCTrack_GL_Rnr::loop_points_and_check_time;
  else 
    vertices_foo = &MCTrack_GL_Rnr::loop_points;


  if(rst_lens->mTrackWidth) glLineWidth(rst_lens->mTrackWidth);
  bool stipple_off_p = false;
  if(rst_lens->mTrackStippleFac) {
    stipple_off_p = (glIsEnabled(GL_LINE_STIPPLE) == false);
    glLineStipple(rst_lens->mTrackStippleFac, rst_lens->mTrackStipplePat);
    glEnable(GL_LINE_STIPPLE);
  }

  A_Rnr* irnr = 0;  
  ZColor col;
  if(rst_lens->mUseSingleCol) {
    col = rst_lens->mSingleCol;
  } else {
    col = rst_lens->GetPdgColor(mMCTrack->mParticle->GetPdgCode());
    if(rst_lens->GetPdgTexture(p->GetPdgCode())) {
      irnr = rd->GetLensRnr(rst_lens->GetPdgTexture(p->GetPdgCode()));
      irnr->PreDraw(rd);
    }
  }

  glColor4fv(col());
  glBegin(GL_LINE_STRIP);
  (this->*vertices_foo)(col);
  glEnd();


  if(rst_lens->mRnrPoints) {
    glColor4fv(col());
    glBegin(GL_POINTS);
    (this->*vertices_foo)(col);
    glEnd();
  }

  if(irnr) irnr->PostDraw(rd);
  if(stipple_off_p) glDisable(GL_LINE_STIPPLE);

}

/**************************************************************************/

void MCTrack_GL_Rnr::make_track(RnrDriver* rd)
{
  MCParticle* p = mMCTrack->mParticle;
  Float_t px = p->Px(), py=p->Py(), pz=p->Pz();  
  MCVertex  mc_v0;
  mc_v0.x = p->Vx();
  mc_v0.y = p->Vy();
  mc_v0.z = p->Vz();
  mc_v0.t = p->T();

  MCTrackRnrStyle* rst_lens  = (MCTrackRnrStyle*) mParticleRMS.lens();
  Float_t max_time = rst_lens->mMaxT * TMath::Power(10, rst_lens->mMaxTScale);
  
   
  if (rst_lens->mMagField && p->GetPDG()->Charge()) {
    Float_t a = 0.2998*rst_lens->mMagField*p->GetPDG()->Charge()/300; // m->cm, 
    MCHelix helix(rst_lens, &mc_v0, TMath::C()*p->P()/p->Energy(), &track_points, a);

    // case 1  
    Bool_t in_bounds = true;
    if( rst_lens->mFitDaughters && (mMCTrack->IsEmpty() == false)){
      list<MCTrack*> dts; 
      mMCTrack->CopyListByGlass<MCTrack>(dts);
      for(list<MCTrack*>::iterator i=dts.begin(); i!=dts.end(); ++i) {
	GLensReadHolder _rlck(*i);
	MCParticle* d = (*i)->mParticle;
	if (in_bounds) {
	  helix.init(TMath::Sqrt(px*px+py*py), pz);
	  in_bounds = helix.loop_to_vertex(px,py,pz, d->Vx(),d->Vy(),d->Vz());
	  if(rst_lens->mFixDaughterTime){
	    d->SetProductionVertex(d->Vx(),d->Vy(),d->Vz(),helix.v.t);
	  }
	  // after reaching daughter birth point reduce momentum
	  px -= d->Px(); py -= d->Py(); pz -= d->Pz();
	} else {
	  if(rst_lens->mFixDaughterTime) {
	    d->SetProductionVertex(d->Vx(),d->Vy(),d->Vz(), max_time+1);
	  }
	}
      }
    }

    //case 2
    if(rst_lens->mFitDecay && p->bDecayed) {
      helix.init(TMath::Sqrt(px*px+py*py), pz);
      helix.loop_to_vertex(px, py, pz, p->fDx, p->fDy, p->fDz);
      //printf("%s decay offset(%f,%f), steps %d \n",mMCTrack->GetName(),helix.x_off, helix.y_off, helix.fN);
    }

    // case 3
    if((rst_lens->mFitDaughters == false && rst_lens->mFitDecay == false) ||
       p->bDecayed == false) 
      {
	helix.init(TMath::Sqrt(px*px + py*py), pz);
	helix.loop_to_bounds(px, py, pz);
      }
  } else {  // *************** LINE ************************
    MCLine line(rst_lens,&mc_v0 , TMath::C()*p->P()/p->Energy(), &track_points);

    // draw line to daughters
    if(rst_lens->mFitDaughters && mMCTrack->IsEmpty() == false){
      list<MCTrack*> dts; 
      mMCTrack->CopyListByGlass<MCTrack>(dts);

      Bool_t inside = true;
      for(list<MCTrack*>::iterator i=dts.begin(); i!=dts.end(); ++i) {
	GLensReadHolder _rlck(*i);
	MCParticle* d = (*i)->mParticle;
        if(inside) {
	  inside = line.in_bounds(d->Vx(),d->Vy(), d->Vz());
	  line.goto_vertex(d->Vx(), d->Vy(), d->Vz());
	  if(rst_lens->mFixDaughterTime){
	    d->SetProductionVertex(d->Vx(), d->Vy(), d->Vz(), line.v.t);
	  }
	  px -= d->Px(); py -= d->Py(); pz -= d->Pz();
	} else {
	  if(rst_lens->mFixDaughterTime){
	    d->SetProductionVertex(d->Vx(), d->Vy(), d->Vz(), max_time + 1);
	  }
	}
      }
    }

    // draw line to the final point if daugter has not reached the boundaries
    if(rst_lens->mFitDecay && p->bDecayed) {
      if(line.in_bounds(p->fDx, p->fDy, p->fDz)) {
	line.goto_vertex(p->fDx, p->fDy, p->fDz);
	return;
      }
    }

    line.goto_bounds(px,py,pz);
  } // Line
}


/**************************************************************************/

void MCTrack_GL_Rnr::loop_points(ZColor& col)
{
  MCTrackRnrStyle* fRnrMod  = (MCTrackRnrStyle*) mParticleRMS.lens();
  Float_t fac = TMath::C()*mMCTrack->GetParticle()->P() /
    (mMCTrack->GetParticle()->Energy()*fRnrMod->mTexFactor);
  for(vector<MCVertex>::iterator i=track_points.begin(); i!=track_points.end(); ++i){
    MCVertex& v = *i;
    glTexCoord2f(v.t*fac + fRnrMod->mTexUCoor, fRnrMod->mTexVCoor);
    glVertex3f(v.x, v.y, v.z);
  }
}

void MCTrack_GL_Rnr::loop_points_and_check_time(ZColor& col)
{
  
  MCTrackRnrStyle* rst_lens  = (MCTrackRnrStyle*) mParticleRMS.lens();
  Float_t max_time = rst_lens->mMaxT * TMath::Power(10, rst_lens->mMaxTScale);
  Float_t min_time = rst_lens->mMinT * TMath::Power(10, rst_lens->mMaxTScale);
  Float_t t_satur  = min_time + rst_lens->mSatur*(max_time - min_time);
  
  Float_t alpha;

  for(vector<MCVertex>::iterator i=track_points.begin(); i!=track_points.end(); ++i) {
    MCVertex& v = *i;
    
    if( ! (v.t < min_time) ) {
      // interpolate vertex to the time interval and break the loop
      if(v.t > max_time) {
	vector<MCVertex>::iterator j = i; --j;
	MCVertex& p = *j;
	MCVertex  m;
	Float_t dt = (max_time - p.t) / (v.t - p.t);
	m.x = p.x + dt*(v.x - p.x);
	m.y = p.y + dt*(v.y - p.y);
	m.z = p.z + dt*(v.z - p.z);
	glColor4fv(rst_lens->mHeadCol());
	glVertex3f(m.x,m.y,m.z);
	return;    
      }
    
      //calculate alpha 
      if(v.t > t_satur) {
	alpha = 1.;
      } else {
	alpha = (v.t - min_time)/(t_satur - min_time);
      }
      glColor4f(col[0], col[1], col[2], col[3]*alpha);
      glVertex3f(v.x,v.y,v.z);
    } // t > tmin
  }
}

