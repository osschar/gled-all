// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TOFSector_GL_Rnr.h"
#include <GL/gl.h>
#include <Rnr/GL/SphereTrings.h>

#include <AliTOFdigit.h>

#include <TMath.h>

/**************************************************************************/

void TOFSector_GL_Rnr::_init()
{}

/**************************************************************************/
void TOFSector_GL_Rnr::Draw(RnrDriver* rd)
{
  obtain_rnrmod(rd, mSegRMS);
  mDRM = (TOFDigRnrMod*) mSegRMS.fRnrMod->fLens;
  ZNode_GL_Rnr::Draw(rd);
}

void TOFSector_GL_Rnr::Render(RnrDriver* rd)
{
  for (Int_t i = 0; i<5; i++){
    RenderPlate(i, rd);
  }
}


void TOFSector_GL_Rnr::RenderPlate(Int_t plate, RnrDriver* rd)
{
  // printf("Render sec %d plate %d \n",mTOFSector->mSecID, plate);
  AliTOFGeometry& geom = *mTOFSector->mInfo->mGeom;  
  Float_t sec_phi = 20*(mTOFSector->mSecID +0.5) - 90;
  Float_t hMw  = geom.StripLength()/2;

  Int_t nstrips;  
  if(plate==0 || plate == 4) 
    nstrips =  geom.NStripC();
  else if(plate==1 || plate == 3) 
    nstrips = geom.NStripB();
  else  
    nstrips = geom.NStripA();

  glPushMatrix();
  glRotatef(sec_phi, 0., 0., 1.);


  Float_t theta,r,x,z;
  for(Int_t strip=0; strip < nstrips; strip++) {
    // TOF geometry changed (Dec 2005, split into subclasses V4, V5).
    // GetStripTheta only defined for V4.
    // This is fix is questionable.
    // theta =  90 - TMath::RadToDeg()*geom.GetStripTheta(plate,strip);     
    theta =  90 - geom.GetAngles(plate,strip);     
    // z = r*TMath::Tan(0.5*TMath::Pi()- geom.GetStripTheta(plate, strip));
    z = r*TMath::Tan(0.5*TMath::Pi() - TMath::DegToRad()*geom.GetAngles(plate, strip));
    r = 0.5*(geom.Rmin()+ geom.Rmax()) + geom.GetHeights(plate,strip);

    glPushMatrix();
    glTranslatef(0, r-0.25, z);
    glRotatef(theta, 1., 0., 0.);
    
    // digits
    AliTOFdigit* d;
    Int_t nds = 0;
    Int_t nd = mTOFSector->mInfo->mDigits->GetEntriesFast();

    if(mDRM->bRnrFlat){
      for(Int_t j=0; j<nd; j++){
	d = (AliTOFdigit*) mTOFSector->mInfo->mDigits->UncheckedAt(j);
	if((d->GetSector() == mTOFSector->mSecID) && (d->GetPlate() == plate) && 
	   (d->GetStrip() == strip) && (d->GetTrack(0) >= 0)){
	  nds++;
	  MkCol(d->GetAdc(),0,140);
	  x = (geom.NpadX()/2. - 0.5 -d->GetPadx())*geom.XPad();
	  z = (0.5 -d->GetPadz())*geom.ZPad();

	  glBegin(GL_LINE_LOOP);
	  glVertex3f(x - geom.XPad()/2, 0.,z - geom.ZPad()/2);
	  glVertex3f(x + geom.XPad()/2, 0.,z - geom.ZPad()/2);
	  glVertex3f(x + geom.XPad()/2, 0.,z + geom.ZPad()/2);
	  glVertex3f(x - geom.XPad()/2, 0.,z + geom.ZPad()/2);
	  glEnd();
	}
      }
    } else {
      for(Int_t j=0; j<nd; j++){
	d = (AliTOFdigit*) mTOFSector->mInfo->mDigits->UncheckedAt(j);
	if((d->GetSector() == mTOFSector->mSecID) && (d->GetPlate() == plate) && 
	   (d->GetStrip() == strip) && (d->GetTrack(0) >= 0)){
	  nds++;
	  MkCol(d->GetAdc(),0,140);
	  x = (geom.NpadX()/2. - 0.5 -d->GetPadx())*geom.XPad();
	  z = (0.5 -d->GetPadz())*geom.ZPad();
	  glPushMatrix();
          glTranslatef(x, 0.,z);
          glRotatef(180,1.,0.,0.);
	  Float_t zs = mDRM->mTdcMinH + mDRM->mTdcScale*TMath::Sqrt(d->GetTdc());
	  glScalef(geom.XPad(), zs,geom.ZPad());
	  SphereTrings::UnitBox();
	  glPopMatrix();
	}
      }
    }

    // printf("%d digits in strip %d\n",nds,strip);
    glBegin(GL_LINE_LOOP);
    if(mDRM->bRnrStripFrame && (nds || mDRM->bRnrEmptyStrip)) {
      glColor4fv(mDRM->mStripCol());
      glVertex3f(-hMw, 0., - geom.ZPad());
      glVertex3f(-hMw, 0., geom.ZPad());
      glVertex3f(hMw, 0.,  geom.ZPad());
      glVertex3f(hMw, 0.,  -geom.ZPad());
    }
    glEnd();
    glPopMatrix();
  } // end loop through stip

  if(mDRM->bRnrModFrame ) {
    Int_t index=nstrips-1;
    Float_t delta =0.;
    if(plate==0)delta = -1. ;
    if(plate==1)delta = -0.5;
    if(plate==3)delta = +0.5;
    if(plate==4)delta = +1. ;

    //Float_t zmax= (geom.Rmin()+2.)*TMath::Tan(geom.GetAngles(plate, index)*TMath::DegToRad())+delta;
    Float_t zmax = (geom.Rmin()+geom.Rmax())*0.5*TMath::Tan(geom.GetAngles(plate, index)*TMath::DegToRad())+delta;
    zmax -= geom.ZPad()*TMath::Cos(geom.GetAngles(plate, index)*TMath::DegToRad());

    index = 0;
    Float_t zmin= (geom.Rmin()+geom.Rmax())*0.5*TMath::Tan(geom.GetAngles(plate, index)*TMath::DegToRad())+delta;
    zmin -= geom.ZPad()*TMath::Cos(geom.GetAngles(plate, index)*TMath::DegToRad());

    glPushMatrix();
    r = (geom.Rmin()+geom.Rmax())/2.;
    glTranslatef(0.,r,0.);    
    glBegin(GL_LINE_LOOP);
    glColor4fv(mDRM->mPlateCol());
    glVertex3f(-hMw, 0., zmax);
    glVertex3f( hMw, 0., zmax);
    glVertex3f( hMw, 0., zmin);
    glVertex3f(-hMw, 0., zmin);
    glEnd();
    glPopMatrix();
  }
  glPopMatrix(); // rotatation by sec
}

