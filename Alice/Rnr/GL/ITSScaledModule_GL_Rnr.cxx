// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ITSScaledModule_GL_Rnr.h"
#include <GL/gl.h>

#include <AliITSdigit.h>
#include <AliITSdigitSPD.h>
#include <AliITSdigitSDD.h>
#include <AliITSdigitSSD.h>
/**************************************************************************/

void ITSScaledModule_GL_Rnr::_init()
{}

/**************************************************************************/
namespace {

  struct ScaledDigit 
  {
    Int_t N;
    Float_t sum;
    Float_t sqr_sum;
    Int_t i,j;

    ScaledDigit() { N = 0; sum = 0; sqr_sum = 0;}
  
    void Dump() {printf("N %d, sum %f, sqr_sum %f",N, sum, sqr_sum);}
  };

}

/**************************************************************************/
void ITSScaledModule_GL_Rnr::Render(RnrDriver* rd)
{
  ITSScaledModule& mM = *mITSScaledModule;

  Int_t Nx,Nz;     // num cells
  Int_t i,j,id;
  Float_t x,z,zo;  
  Float_t dpx,dpz; // cell sizes
  map<Int_t, ScaledDigit> dmap;  
  Bool_t above_treshold = false;

  Int_t sx,sz;

  
  if(mDRM->mDigitW) glLineWidth(mDRM->mDigitW);
  TClonesArray* digits  = mITSModule->mInfo->GetDigits(mITSModule->mID, mITSModule->mDetID );
  Int_t  ndigits = digits->GetEntriesFast(); 
  switch(mITSModule->mDetID) {
  case 0: {
    sz = mM.mInfo->mSPDScaleZ[mDRM->mScale-1];
    sx = mM.mInfo->mSPDScaleX[mDRM->mScale-1];
    Nz  = Int_t(mM.mInfo->mSegSPD->Npz()/sz);
    Nx  = Int_t(mM.mInfo->mSegSPD->Npx()/sx);
    dpz = 2*mM.mDz/Nz;
    dpx = 2*mM.mDx/Nx;

    AliITSdigitSPD *od=0;
    for (Int_t k=0; k<ndigits; k++) {
      od=(AliITSdigitSPD*)digits->UncheckedAt(k);
      mM.mInfo->GetSPDLocalZ(od->GetCoord1(),zo);
      i = Int_t((zo+ mM.mDz)/dpz);
      j = Int_t(od->GetCoord2()/sx);
      id = j*Nx + i; 
      dmap[id].i = i; dmap[id].j = j;
      dmap[id].N++;
      dmap[id].sum += od->GetSignal();
      dmap[id].sqr_sum += od->GetSignal()*od->GetSignal();
    }
  
    glBegin (GL_QUADS);
    for(map<Int_t,ScaledDigit>::iterator k=dmap.begin(); k!=dmap.end(); ++k) {
      id = k->first;
      switch(mDRM->mStatType) {
      case ITSDigRnrMod::ST_Occup:   MkCol(k->second.N*1./(Nz*Nx),0.,mDRM->mMaxOcc);   break;
      case ITSDigRnrMod::ST_Average: MkCol((k->second.sum*1.)/(1.*k->second.N),0.,mDRM->mMaxAverage); break;
      case ITSDigRnrMod::ST_Rms:     MkCol(TMath::Sqrt(k->second.sqr_sum)/(1.*k->second.N),0.,mDRM->mMaxRms); break;    
      }
      i = k->second.i; 
      j = k->second.j; 
      z = dpz*(i+0.5) - mITSScaledModule->mDz;
      x = dpx*(j+0.5) - mITSScaledModule->mDx;

      glVertex3f(x - dpx/2, 0, z-dpz/2);
      glVertex3f(x - dpx/2, 0, z+dpz/2);
      glVertex3f(x + dpx/2, 0, z+dpz/2);
      glVertex3f(x + dpx/2, 0, z-dpz/2);
    }  
    glEnd();
    break;
  } // end case 0
  case 1: {
    sz = mM.mInfo->mSDDScaleZ[mDRM->mScale-1];
    sx = mM.mInfo->mSDDScaleX[mDRM->mScale-1];   
    Nz  = Int_t(mM.mInfo->mSegSDD->Npz()/sz);
    Nx  = Int_t(mM.mInfo->mSegSDD->Npx()/sx);
    dpz = 2*mM.mDz/Nz;
    dpx = 2*mM.mDx/Nx;

    AliITSdigitSDD *od=0;
    for (Int_t k=0; k<ndigits; k++) {
      od=(AliITSdigitSDD*)digits->UncheckedAt(k);
      if(od->GetSignal() > mDRM->mSDDTreshold){
	mM.mInfo->mSegSDD->DetToLocal(od->GetCoord2(), od->GetCoord1(),x,z);
	z+= mM.mDz;
	x+= mM.mDx;
	i = Int_t(z/dpz);
	j = Int_t(x/dpx);
	// printf("Mod %d coord %d,%d out of %d,%d :: ORIG coord %d,%d out of %d,%d \n",mITSModule->mDetID,
	//	     i,j,Nz,Nx,od->GetCoord1(),od->GetCoord2(),mM.mInfo->mSegSDD->Npz(),mM.mInfo->mSegSDD->Npx());
	id = j*Nx + i; 
	dmap[id].i = i; dmap[id].j = j;
	dmap[id].N++;
	dmap[id].sum += od->GetSignal();
	dmap[id].sqr_sum += od->GetSignal()*od->GetSignal();
      }
    }

    glBegin (GL_QUADS);
    for(map<Int_t,ScaledDigit>::iterator k=dmap.begin(); k!=dmap.end(); ++k) {
      id = k->first;
      i = k->second.i; 
      j = k->second.j; 
      //printf("calculatede %d,%d rememberd %d,%d \n",id % Nx, Int_t(id/Nx),i,j);
      switch(mDRM->mStatType) {
      case ITSDigRnrMod::ST_Occup:   MkCol(k->second.N*1./(Nz*Nx),0.,mDRM->mMaxOcc);   break;
      case ITSDigRnrMod::ST_Average: MkCol((k->second.sum*1.)/(1.*k->second.N),0.,mDRM->mMaxAverage); break;
      case ITSDigRnrMod::ST_Rms:     MkCol(TMath::Sqrt(k->second.sqr_sum)/(1.*k->second.N),0.,mDRM->mMaxRms); break;    
      }
      z = dpz*(i+0.5) - mM.mDz;
      x = dpx*(j+0.5) - mM.mDx;

      glVertex3f(x - dpx/2, 0, z-dpz/2);
      glVertex3f(x - dpx/2, 0, z+dpz/2);
      glVertex3f(x + dpx/2, 0, z+dpz/2);
      glVertex3f(x + dpx/2, 0, z-dpz/2);
    }
    glEnd();
    break;
  }// end case 1
  case 2: {
    sx = mM.mInfo->mSSDScale[mDRM->mScale-1];   
    Nx  = Int_t(mM.mInfo->mSegSSD->Npx()/sx);
    dpz = 2*mM.mDz/Nz;
    dpx = 2*mM.mDx/Nx;

    AliITSdigitSSD *od=0;
    for (Int_t k=0; k<ndigits; k++) {
      od=(AliITSdigitSSD*)digits->UncheckedAt(k);
      if(od->GetSignal() > mDRM->mSSDTreshold){
	if(od->GetCoord1() == 1) 
	  i = 1; // p side
	else 
	  i= -1; // n side
	j = Int_t(od->GetCoord2()/sx);
	id = j*i; 
        //printf("orig digit %d,%d scaled %d,%d \n",od->GetCoord1(),od->GetCoord2(),i,j);
	dmap[id].i = i; dmap[id].j = j;
	dmap[id].N++;
	dmap[id].sum += od->GetSignal();
	dmap[id].sqr_sum += od->GetSignal()*od->GetSignal();
      }
    }

    Float_t ap,an,a;
    mM.mInfo->mSegSSD->Angles(ap,an);
    glBegin (GL_LINES);
    for(map<Int_t,ScaledDigit>::iterator k=dmap.begin(); k!=dmap.end(); ++k) {
      id = k->first;
      i = k->second.i; 
      j = k->second.j; 
      Int_t tj = Int_t((j+0.5)*sx);
      if (i > 0){
	mM.mInfo->mSegSSD->DetToLocal(tj,i,x,z);
	a = ap;
      }
      else {
	mM.mInfo->mSegSSD->DetToLocal(tj,0,x,z);
	a = -an;
      }
      switch(mDRM->mStatType) {
      case ITSDigRnrMod::ST_Occup:   MkCol(k->second.N*1./(2*Nx),0.,mDRM->mMaxOcc);   break;
      case ITSDigRnrMod::ST_Average: MkCol((k->second.sum*1.)/(1.*k->second.N),0.,mDRM->mMaxAverage); break;
      case ITSDigRnrMod::ST_Rms:     MkCol(TMath::Sqrt(k->second.sqr_sum)/(1.*k->second.N),0.,mDRM->mMaxRms); break;    
      }
      glVertex3f( x-TMath::Tan(a)*mM.mDz,0,-mM.mDz);
      glVertex3f( x+TMath::Tan(a)*mM.mDz,0,mM.mDz);
    }
    glEnd();
    break;
  } // end case 2
  } //end switch

  if(! dmap.empty())above_treshold = true;
  // Frame
  x = mITSScaledModule->mDx;
  z = mITSScaledModule->mDz;
  if(mDRM->bRnrFrame  && above_treshold) {
    if(mDRM->mFrameW) glLineWidth(mDRM->mFrameW);
    glBegin (GL_LINE_LOOP);
    glColor4fv(mDRM->mFrameCol());
    glVertex3f( x, 0,  z);
    glVertex3f(-x, 0,  z);
    glVertex3f(-x, 0, -z);
    glVertex3f( x, 0, -z);
    glEnd();
  }
  // printf("rnr scale (%d,%d) from RM %d\n", sx, sz, mDRM->mScale);
}

