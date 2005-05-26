// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZParticle_GL_Rnr.h"
#include "HelixLineRnr.h"
#include <Glasses/ZParticleRnrStyle.h>
#include <FL/gl.h>

/**************************************************************************/
void ZParticle_GL_Rnr::_init()
{
  mParticleRMS.fFid = ZParticleRnrStyle::FID();
}

/**************************************************************************/

void ZParticle_GL_Rnr::PreDraw(RnrDriver* rd)
{
  ZGlass_GL_Rnr::PreDraw(rd);
  rd->PushPM(mZParticle);
}

void ZParticle_GL_Rnr::Draw(RnrDriver* rd)
{
  obtain_rnrmod(rd, mParticleRMS);
  ZNode_GL_Rnr::Draw(rd);
}

void ZParticle_GL_Rnr::PostDraw(RnrDriver* rd)
{
  rd->PopPM();
  ZGlass_GL_Rnr::PostDraw(rd);
}

/**************************************************************************/

void ZParticle_GL_Rnr::Render(RnrDriver* rd)
{
  RNRDRIVER_GET_RNRMOD(srm, rd, ZParticleRnrStyle);
  ZParticleRnrStyle* rst_lens  = (ZParticleRnrStyle*) srm->fLens;
  // !!! temporary hardcode value of magnetic field
  Float_t B = rst_lens->mMagField/10;// kGauss->T

  MCParticle* p = mZParticle->mParticle;
  Float_t vx=p->Vx(), vy=p->Vy(), vz=p->Vz();
  Float_t px = p->Px(), py=p->Py(), pz=p->Pz();  
  // check P cut off
  if (p->P() < rst_lens->mMinP) return;
  //  printf("ZParticle_GL_Rnr::Render THETA %f \n",p->Theta());
  if(TMath::RadToDeg()*p->Theta() < (rst_lens->mTheta - rst_lens->mThetaOff) || 
     TMath::RadToDeg()*p->Theta() > (rst_lens->mTheta + rst_lens->mThetaOff)  )return;
  // check boundaries
  if(TMath::Abs(vz)>rst_lens->mMaxZ || (vx*vx + vy*vy) > (rst_lens->mMaxR)*(rst_lens->mMaxR)) return;

  //render particle as point
  glPointSize(rst_lens->mVertexSize);
  glBegin(GL_POINTS);
  glColor4fv(rst_lens->mVertexColor());
  glVertex3f(vx,vy,vz);
  glEnd();

  // show particle momentum
  if ( rst_lens->mRnrP) {
    glLineWidth(2*rst_lens->mTrackWidth);
    glBegin(GL_LINES);
    glColor4fv(rst_lens->mPColor());
    glVertex3f(vx,vy,vz);
    Float_t ps = rst_lens->mPMinLen + TMath::Log10(1 + p->P()/rst_lens->mPScale);
    TVector3 v(px,py,pz);
    v = ps*v.Unit();
    glVertex3f(v.X() + vx, v.Y() + vy, v.Z() + vz);
    glEnd();
  }

  // show tracks tracks 
  if(rst_lens->mMaxR != 0) {
    glLineWidth(rst_lens->mTrackWidth);
    ZColor c = rst_lens->GetPdgColor( p->GetPdgCode());
    glColor4fv(c());

    A_Rnr* irnr = 0;
    if(rst_lens->GetPdgTexture( p->GetPdgCode())) {
      irnr = rd->GetLensRnr(rst_lens->GetPdgTexture( p->GetPdgCode()));
      irnr->PreDraw(rd);
    }

    if(p->GetPDG() == 0){
      printf("ERROR, can\t find PDG code for %d\n",p->GetPdgCode()); 
      return;
    }

    if (B && p->GetPDG()->Charge()) {
      Float_t a = 0.2998*B*3*p->GetPDG()->Charge()/1000; // m->mm
      Helix helix(a, rst_lens);

      // case 1  
      if( rst_lens->mFitDaughters && (mZParticle->IsEmpty() == false)){
	list<ZParticle*> dts; 
	mZParticle->CopyByGlass<ZParticle*>(dts);
	for(list<ZParticle*>::iterator i=dts.begin(); i!=dts.end(); ++i) {
	  GLensReadHolder _rlck(*i);
	  const MCParticle* d = (*i)->GetParticle();
	  helix.init( TMath::Sqrt(px*px+py*py), pz);
	  Bool_t bound = helix.loop_from_to(vx,vy,vz,px,py,pz,
					    d->Vx(),d->Vy(),d->Vz());
	  if(bound = false) goto fin;
	  // after reach daughter birt point reduce momentum
	  px -= d->Px(); py -= d->Py(); pz -= d->Pz();
	}
      }

      //case 2
      if(rst_lens->mFitDecay &&  p->bDecayed ){
	helix.init(TMath::Sqrt(px*px+py*py), pz);
	helix.loop_from_to(vx, vy, vz, px, py, pz,
			   p->fDx, p->fDy, p->fDz);
	//printf("%s decay offset(%f,%f), steps %d \n",mZParticle->GetName(),helix.x_off, helix.y_off, helix.fN);
      }

      // case 3
      if((rst_lens->mFitDaughters == false && rst_lens->mFitDecay == false) || p->bDecayed == false){
	helix.init( TMath::Sqrt(px*px+py*py), pz);
	helix.loop_to_bounds(vx, vy, vz, px, py, pz);
      }
     
      if(rst_lens->mFitDecay == false &&  p->bDecayed == true){
	helix.init( TMath::Sqrt(px*px+py*py), pz);
	helix.loop_to_bounds(vx, vy, vz, px, py, pz);
      }

    } else {

      Line line;
      line.fRnrMod=rst_lens;
      Float_t Tb = line.get_bounds_time(vx, vy, vz, px, py, pz);

      // draw line to daughters
      if(rst_lens->mFitDaughters ){
        if( mZParticle->IsEmpty() == false){
          list<ZParticle*> dts; 
          mZParticle->CopyByGlass<ZParticle*>(dts);
          for(list<ZParticle*>::iterator i=dts.begin(); i!=dts.end(); ++i) {
            GLensReadHolder _rlck(*i);
            const MCParticle* d = (*i)->GetParticle();

            if(line.in_bounds(d->Vx(),d->Vy(), d->Vz()) == false){
              line.draw_to(vx,vy,vz, vx + px*Tb, vy + py*Tb, vz + pz*Tb);
              goto fin;
            }

            line.draw_to(vx,vy,vz,d->Vx(), d->Vy(), d->Vz());
            // printf("%s draw daughter %s\n", mZParticle->GetName(), d->GetName());
            px -= d->Px();  py -= d->Py(); pz -= d->Pz();
            Tb = line.get_bounds_time(vx, vy, vz, px, py, pz);
          }
        }
      }  

      // draw line to the final point if daugter has not reached the boundaries
      if(rst_lens->mFitDecay){
        if(p->bDecayed) {
          if(line.in_bounds(p->fDx,p->fDy, p->fDz)){
            line.draw_to(vx,vy,vz,p->fDx,p->fDy, p->fDz);
            goto fin;
          }
          else {
            Tb = line.get_bounds_time(vx, vy, vz, px, py, pz);
            line.draw_to(vx,vy,vz, vx + px*Tb, vy + py*Tb, vz + pz*Tb);
            goto fin;
          }
        }
      }

      // case:  fit disabled, not decayed
      line.draw_to(vx, vy, vz, vx+px*Tb, vy+py*Tb, vz+pz*Tb);
   
    } // if neutral
  fin:
    if(irnr) irnr->PostDraw(rd);
  } // if rnr tracks
} //Render func
