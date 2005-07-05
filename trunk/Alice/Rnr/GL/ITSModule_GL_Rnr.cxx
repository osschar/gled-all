// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ITSModule_GL_Rnr.h"
#include <FL/gl.h>

#include <AliITSdigitSPD.h>
#include <AliITSdigitSDD.h>
#include <AliITSdigitSSD.h>
/**************************************************************************/

void ITSModule_GL_Rnr::_init()
{}

/**************************************************************************/

void ITSModule_GL_Rnr::Draw(RnrDriver* rd)
{
  obtain_rnrmod(rd, mSegRMS);
  mDRM = (ITSDigRnrMod*) mSegRMS.fRnrMod->fLens;
  ZNode_GL_Rnr::Draw(rd);
}

void ITSModule_GL_Rnr::Render(RnrDriver* rd)
{
  Float_t x = mITSModule->mDx;
  Float_t z = mITSModule->mDz;
  Bool_t above_treshold = false;
 
  // digits
  TClonesArray *digits;
  Int_t ndigits;
  Float_t dpx,dpz; 
  Int_t i,j;
  digits = mITSModule->mInfo->GetDigits(mITSModule->mID, mITSModule->mDetID );
  ndigits=digits->GetEntriesFast(); 
  // printf("%d digits in %d\n", ndigits,mITSModule->mID);

  switch(mITSModule->mDetID) {
  case 0: {
    above_treshold = true;
    AliITSsegmentationSPD* seg =  mITSModule->mInfo->mSegSPD; 
    AliITSdigitSPD *d=0;

    glBegin (GL_QUADS);
    for (Int_t k=0; k<ndigits; k++) {
      d=(AliITSdigitSPD*)digits->UncheckedAt(k);
      j = d->GetCoord1();
      i = d->GetCoord2();
      MkCol(1, 0,1);
      x = -seg->Dx()/2 + seg->Dpx(0) *i;
      x *= 0.0001;
      mITSModule->mInfo->GetSPDLocalZ(j,z);
      dpx = seg->Dpx(i)*0.0001;
      dpz = seg->Dpz(j)*0.0001;

      glVertex3f( x,0,z);
      glVertex3f( x,0,z+dpz);
      glVertex3f( x+dpx,0,z+dpz);
      glVertex3f( x+dpx,0,z);
    }
    glEnd();
    break;
  }
  case 1:{
    AliITSsegmentationSDD* seg =  mITSModule->mInfo->mSegSDD; 
    AliITSdigitSDD *d=0;

    glBegin (GL_QUADS);
    for (Int_t k=0; k<ndigits; k++) {
      d=(AliITSdigitSDD*)digits->UncheckedAt(k);
      if(d->GetSignal() > mDRM->mSDDTreshold){
	j = d->GetCoord1();
	i = d->GetCoord2();
	above_treshold = true;
	MkCol(d->GetSignal(), mDRM->mSDDTreshold, mDRM->mSDDMaxVal);
	seg->DetToLocal(i,j,x,z);

	dpx = seg->Dpx(i)*0.0001;
	dpz = seg->Dpz(j)*0.0001;

	glVertex3f( x,0,z);
	glVertex3f( x,0,z+dpz);
	glVertex3f( x+dpx,0,z+dpz);
	glVertex3f( x+dpx,0,z);
      }
    }
    glEnd();
    break;
  }
  case 2:{
    AliITSsegmentationSSD* seg =  mITSModule->mInfo->mSegSSD; 
    AliITSdigitSSD *d=0;

    glBegin (GL_LINES);
    for (Int_t k=0; k<ndigits; k++) {
      d=(AliITSdigitSSD*)digits->UncheckedAt(k);
      if(d->GetSignal() > mDRM->mSSDTreshold){
	above_treshold = true;
	j = d->GetCoord1();
	i = d->GetCoord2();
	seg->DetToLocal(i,j,x,z);
	// printf("Coord i,j %f,%f  x,z %f,%f\n",i,j,x,z);
	dpx = seg->Dpx(i)*0.0001;
	dpz = seg->Dpz(j)*0.0001;
	Float_t ap,an,a;
	seg->Angles(ap,an);
	if( d->GetCoord1() == 1) {
	  MkCol(d->GetSignal(), mDRM->mSSDTreshold, mDRM->mSSDMaxVal);
	  a = ap;
	}
	else {
	  MkCol(d->GetSignal(), mDRM->mSSDTreshold, mDRM->mSSDMaxVal);
	  a = -an;
	}
     
	glVertex3f( x,0,-mITSModule->mDz);
	glVertex3f( x+TMath::Tan(a)*mITSModule->mDz*2,0,mITSModule->mDz);
      }
    }
    glEnd();
    break;
  }
  }

  //frame
  x = mITSModule->mDx;
  z = mITSModule->mDz;
  if(mDRM->bRnrFrame && above_treshold){
    glBegin (GL_QUADS);
    glColor4fv(mDRM->mFrameCol());
    glVertex3f( x, 0,  z);
    glVertex3f(-x, 0,  z);
    glVertex3f(-x, 0, -z);
    glVertex3f( x, 0, -z);
    glEnd();
  }
}
