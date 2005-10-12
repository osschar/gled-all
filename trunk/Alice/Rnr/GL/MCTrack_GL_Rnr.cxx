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

  MCTrackRnrStyle& RS = * (MCTrackRnrStyle*) mParticleRMS.lens();

  Float_t vx = p->Vx(), vy=p->Vy(), vz=p->Vz();
  Float_t px = p->Px(), py=p->Py(), pz=p->Pz(); 

  if (p->P() < RS.mMinP) show_p = false;
  if(TMath::Abs(TMath::RadToDeg()*p->Theta() - RS.mTheta) >
     RS.mThetaOff)       show_p = false;
  if(TMath::Abs(dphi_deg(TMath::RadToDeg()*p->Phi(), RS.mPhi)) >
     RS.mPhiOff)         show_p = false;

  
  if(RS.mCheckT && p->T() > RS.mMaxAT)
    show_p = false;

  if(p->GetPDG() == 0) show_p = false;

  // check boundaries
  if(TMath::Abs(vz) > RS.mMaxZ || (vx*vx + vy*vy) > (RS.mMaxR)*(RS.mMaxR)) 
    show_p = false;

  if(RS.mForceVisParents && mMCTrack->bRnrElements != show_p) {
    mMCTrack->SetRnrElements(show_p);
  } 

  if(show_p == false)
    return;

  //render particle as point
  if(RS.mRnrV) {
    if(p->T() > RS.mMinAT) {
      glPointSize(RS.mVertexSize);
      glBegin(GL_POINTS);
      glColor4fv(RS.mVertexColor());
      glVertex3f(vx,vy,vz);
      glEnd();
    }
  }
  
  // show particle momentum
  if(RS.mRnrP) {
    if(RS.mTrackWidth) glLineWidth(2*RS.mTrackWidth);
    glBegin(GL_LINES);
    glColor4fv(RS.mPColor());
    glVertex3f(vx,vy,vz);
    Float_t ps = RS.mPMinLen + TMath::Log10(1 + p->P()/RS.mPScale);
    TVector3 v(px,py,pz);
    v = ps*v.Unit();
    glVertex3f(v.X() + vx, v.Y() + vy, v.Z() + vz);
    glEnd();
  }

  // calculate points and save them to vector
  if(mStampPointCalc < RS.mStampPointCalcReq) {
    track_points.clear();
    make_track(rd);
    mStampPointCalc = RS.mStampPointCalcReq;
  }

  if(RS.mCheckT)
    vertices_foo = &MCTrack_GL_Rnr::loop_points_and_check_time;
  else 
    vertices_foo = &MCTrack_GL_Rnr::loop_points;


  if(RS.mTrackWidth) glLineWidth(RS.mTrackWidth);
  bool stipple_off_p = false;
  if(RS.mTrackStippleFac) {
    stipple_off_p = (glIsEnabled(GL_LINE_STIPPLE) == false);
    glLineStipple(RS.mTrackStippleFac, RS.mTrackStipplePat);
    glEnable(GL_LINE_STIPPLE);
  }

  A_Rnr* irnr = 0;  
  ZColor col;
  if(RS.mUseSingleCol) {
    col = RS.mSingleCol;
  } else {
    col = RS.GetPdgColor(mMCTrack->mParticle->GetPdgCode());
    if(RS.GetPdgTexture(p->GetPdgCode())) {
      irnr = rd->GetLensRnr(RS.GetPdgTexture(p->GetPdgCode()));
      irnr->PreDraw(rd);
    }
  }

  glColor4fv(col());
  glBegin(GL_LINE_STRIP);
  (this->*vertices_foo)(col);
  glEnd();


  if(RS.mRnrPoints) {
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

  MCTrackRnrStyle& RS  = * (MCTrackRnrStyle*) mParticleRMS.lens();
   
  if (RS.mMagField && p->GetPDG()->Charge()) {
    Float_t a = 0.2998*RS.mMagField*p->GetPDG()->Charge()/300; // m->cm
    MCHelix helix(&RS, &mc_v0, TMath::C()*p->P()/p->Energy(), &track_points, a); //m->cm

    // case 1  
    Bool_t in_bounds = true;
    if( RS.mFitDaughters && (mMCTrack->IsEmpty() == false)){
      list<MCTrack*> dts; 
      mMCTrack->CopyListByGlass<MCTrack>(dts);
      for(list<MCTrack*>::iterator i=dts.begin(); i!=dts.end(); ++i) {
	GLensReadHolder _rlck(*i);
	MCParticle* d = (*i)->mParticle;
	if (in_bounds) {
	  helix.init(TMath::Sqrt(px*px+py*py), pz);
	  in_bounds = helix.loop_to_vertex(px,py,pz, d->Vx(),d->Vy(),d->Vz());
	  if(RS.mFixDaughterTime){
	    d->SetProductionVertex(d->Vx(),d->Vy(),d->Vz(),helix.v.t);
	  }
	  // after reaching daughter birth point reduce momentum
	  px -= d->Px(); py -= d->Py(); pz -= d->Pz();
	} else {
	  if(RS.mFixDaughterTime) {
	    d->SetProductionVertex(d->Vx(),d->Vy(),d->Vz(), RS.mMaxAT + 1);
	  }
	}
      }
    }

    //case 2
    if(RS.mFitDecay && p->bDecayed) {
      helix.init(TMath::Sqrt(px*px+py*py), pz);
      helix.loop_to_vertex(px, py, pz, p->fDx, p->fDy, p->fDz);
      //printf("%s decay offset(%f,%f), steps %d \n",mMCTrack->GetName(),helix.x_off, helix.y_off, helix.fN);
    }

    // case 3
    if((RS.mFitDaughters == false && RS.mFitDecay == false) ||
       p->bDecayed == false) 
      {
	helix.init(TMath::Sqrt(px*px + py*py), pz);
	helix.loop_to_bounds(px, py, pz);
      }
  } else {  // *************** LINE ************************
    MCLine line(&RS, &mc_v0 , TMath::C()*p->P()/p->Energy(), &track_points); // m->cm

    // draw line to daughters
    if(RS.mFitDaughters && mMCTrack->IsEmpty() == false){
      list<MCTrack*> dts; 
      mMCTrack->CopyListByGlass<MCTrack>(dts);

      Bool_t inside = true;
      for(list<MCTrack*>::iterator i=dts.begin(); i!=dts.end(); ++i) {
	GLensReadHolder _rlck(*i);
	MCParticle* d = (*i)->mParticle;
        if(inside) {
	  inside = line.in_bounds(d->Vx(),d->Vy(), d->Vz());
	  line.goto_vertex(d->Vx(), d->Vy(), d->Vz());
	  if(RS.mFixDaughterTime){
	    d->SetProductionVertex(d->Vx(), d->Vy(), d->Vz(), line.v.t);
	  }
	  px -= d->Px(); py -= d->Py(); pz -= d->Pz();
	} else {
	  if(RS.mFixDaughterTime){
	    d->SetProductionVertex(d->Vx(), d->Vy(), d->Vz(), RS.mMaxAT + 1);
	  }
	}
      }
    }

    // draw line to the final point if daugter has not reached the boundaries
    if(RS.mFitDecay && p->bDecayed) {
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
  MCTrackRnrStyle* rs = (MCTrackRnrStyle*) mParticleRMS.lens();
  Float_t fac = TMath::C()*mMCTrack->GetParticle()->P() /
    (mMCTrack->GetParticle()->Energy()*rs->mTexFactor);
  for(vector<MCVertex>::iterator i=track_points.begin(); i!=track_points.end(); ++i){
    MCVertex& v = *i;
    glTexCoord2f(v.t*fac + rs->mTexUCoor, rs->mTexVCoor);
    glVertex3f(v.x, v.y, v.z);
  }
}


/**************************************************************************/
/**************************************************************************/

inline int MCTrack_GL_Rnr::find_index(int start, float time)
{
  int j = start;
  while(track_points[j + 1].t < time) ++j;
  return j;
}

inline void MCTrack_GL_Rnr::set_color(Float_t time, ZColor col) 
{
  MCTrackRnrStyle& RS  = * (MCTrackRnrStyle*) mParticleRMS.lens();
  Float_t alpha = 1.;

  if(time > RS.mHeadAT) {
    Float_t xx = (time - RS.mHeadAT) / (RS.mHeadS * RS.mDeltaAT);
    col *= 1 - xx;
    col[0] += xx * RS.mHeadCol[0];
    col[1] += xx * RS.mHeadCol[1];
    col[2] += xx * RS.mHeadCol[2];
    col[3] += xx * RS.mHeadCol[3];
  }
    
  if(time < RS.mAlphaAT){
    alpha = (time - RS.mMinAT) / (RS.mAlphaS * RS.mDeltaAT);
  }
  glColor4f(col[0], col[1], col[2], col[3]*alpha);  
}

inline void MCTrack_GL_Rnr::mark_vertex(Int_t idx, ZColor& orig_col) 
{
  MCVertex& v = track_points[idx];
  set_color(v.t, orig_col);
  glVertex3f(v.x,v.y,v.z);
}

inline void MCTrack_GL_Rnr::interpolate_vertex(Int_t idx, Float_t time, ZColor& orig_col) 
{
  MCVertex m;
  MCVertex& v = track_points[idx];
  MCVertex& p = track_points[idx + 1];

  Float_t dt = (p.t - time) / (p.t - v.t);
  
  m.x = p.x - dt*(p.x - v.x);
  m.y = p.y - dt*(p.y - v.y);
  m.z = p.z - dt*(p.z - v.z);

  set_color(time, orig_col);
  glVertex3f(m.x,m.y,m.z);
}

/**************************************************************************/

void MCTrack_GL_Rnr::loop_points_and_check_time(ZColor& orig_col)
{
  MCTrackRnrStyle& RS  = * (MCTrackRnrStyle*) mParticleRMS.lens();

  Int_t N =  track_points.size();
  
  //return if particle not lived to mMinAT
  if(track_points[N-1].t <  RS.mMinAT) return;

  Float_t t, t1, t2;
  Int_t   i;

  if(RS.mAlphaAT > RS.mHeadAT) {
    t1 = RS.mHeadAT; t2 = RS.mAlphaAT;
  } else {
    t2 = RS.mHeadAT; t1 = RS.mAlphaAT;
  }

  // case particle born before mMinAT
  if(track_points[0].t < RS.mMinAT) {
    i = find_index(0, RS.mMinAT);
    t = RS.mMinAT;
    interpolate_vertex(i, t, orig_col);
  } else {
    i = 0;
    t = track_points[i].t;
    mark_vertex(i, orig_col);
  }

  if(t1 > t && t1 < track_points[N-1].t) {
    int j = find_index(i, t1);
    for(int k=i+1; k<=j; ++k)
      mark_vertex(k, orig_col);
    interpolate_vertex(j, t1, orig_col);
    i = j;
    t = t1;
  }

  if(t2 > t && t2 < track_points[N-1].t) {
    int j = find_index(i, t2);
    for(int k=i+1; k<=j; ++k)
      mark_vertex(k, orig_col);
    interpolate_vertex(j, t2, orig_col);
    i = j;
    t = t2;
  }

  if(RS.mMaxAT < track_points[N-1].t) {
    int j = find_index(i, RS.mMaxAT);
    for(int k=i+1; k<=j; ++k)
      mark_vertex(k, orig_col);
    interpolate_vertex(j, RS.mMaxAT, orig_col);
  } else {
    for(int k=i+1; k<N; ++k)
      mark_vertex(k, orig_col);
  }

}

/**************************************************************************/

/*
    Int_t get_timeidx(Float_t time, Int_t min_idx, Int_t max_idx){
    while(max_idx - min_idx != 1){
    Int_t idx = Int_t((min_idx + max_idx)/2);
    if( time > track_points[idx].t) {
    min_idx = idx;
    max_idx = max_idx;
    }
    else {
    min_idx = min_idx;
    max_idx = idx;
    }
    get_timeidx(time, min_idx, max_idx);
    return max_idx;
    }
*/
