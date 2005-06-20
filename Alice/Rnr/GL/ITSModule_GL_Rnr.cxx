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
{
}

/**************************************************************************/



void ITSModule_GL_Rnr::Draw(RnrDriver* rd)
{
  Float_t x = mITSModule->mDx;
  Float_t z = mITSModule->mDz;

  //frame
  glBegin (GL_QUADS);
  glColor4fv(mITSModule->mFrameCol());
  glVertex3f( x, 0,  z);
  glVertex3f(-x, 0,  z);
  glVertex3f(-x, 0, -z);
  glVertex3f( x, 0, -z);
  glEnd();

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
    AliITSsegmentationSPD* seg =  mITSModule->mInfo->mSegSPD; 
    AliITSdigitSPD *d=0;

    glBegin (GL_QUADS);
    glColor4fv(mITSModule->mDigCol());
    for (Int_t k=0; k<ndigits; k++) {
      d=(AliITSdigitSPD*)digits->UncheckedAt(k);
      j = d->GetCoord1();
      i = d->GetCoord2();

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
    glColor4fv(mITSModule->mDigCol());
    for (Int_t k=0; k<ndigits; k++) {
      d=(AliITSdigitSDD*)digits->UncheckedAt(k);
      j = d->GetCoord1();
      i = d->GetCoord2();
      seg->DetToLocal(i,j,x,z);

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
  case 2:{
    AliITSsegmentationSSD* seg =  mITSModule->mInfo->mSegSSD; 
    AliITSdigitSSD *d=0;

    glBegin (GL_LINES);
    for (Int_t k=0; k<ndigits; k++) {
      d=(AliITSdigitSSD*)digits->UncheckedAt(k);
      j = d->GetCoord1();
      i = d->GetCoord2();
      seg->DetToLocal(i,j,x,z);
      // printf("Coord i,j %f,%f  x,z %f,%f\n",i,j,x,z);
      dpx = seg->Dpx(i)*0.0001;
      dpz = seg->Dpz(j)*0.0001;
      Float_t ap,an,a;
      seg->Angles(ap,an);
      if( d->GetCoord1() == 1) {
	glColor4fv(mITSModule->mSSDPCol());
	a = ap;
      }
      else {
	glColor4fv(mITSModule->mSSDNCol());
	a = -an;
      }
      glVertex3f( x,0,-mITSModule->mDz);
      glVertex3f( x+TMath::Tan(a)*mITSModule->mDz*2,0,mITSModule->mDz);
    }
    glEnd();
    break;
  }
  }
}
