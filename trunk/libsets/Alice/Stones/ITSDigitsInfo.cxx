// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ITSDigitsInfo
//
//

#include "ITSDigitsInfo.h"
#include <AliITSdigit.h>
#include <AliITSdigitSPD.h>

#include <Gled/GledTypes.h>

#include <TFile.h>

ClassImp(ITSDigitsInfo)

/**************************************************************************/

void ITSDigitsInfo::_init()
{
  mDataDir = ".";
  mTree = 0;
  mGeom = 0;
  mSegSPD = 0;
  mSegSDD = 0;
  mSegSSD = 0;
}

/**************************************************************************/

ITSDigitsInfo:: ~ITSDigitsInfo() 
{
  for(map<Int_t, TClonesArray*>::iterator j=mSPDmap.begin(); j!=mSPDmap.end(); ++j) {
    delete j->second;
  }
  for(map<Int_t, TClonesArray*>::iterator j=mSDDmap.begin(); j!=mSDDmap.end(); ++j) {
    delete j->second;
  }
  for(map<Int_t, TClonesArray*>::iterator j=mSSDmap.begin(); j!=mSSDmap.end(); ++j) {
    delete j->second;
  }
  delete mSegSPD; delete mSegSDD; delete mSegSSD; 
  delete mGeom;
  delete mTree;
}

/**************************************************************************/

void ITSDigitsInfo::SetData(const Text_t* data_dir, Int_t event)
{
  static const Exc_t _eh("ITSDigitsInfo::SetData ");

  mDataDir = "";
  mEvent   = -1;

  if(mGeom == 0) {
    mGeom = new AliITSgeom();
    mGeom->ReadNewFile("ITSgeometry.det");
    if(mGeom == 0)
      throw(_eh + GForm("can not load ITS geometry \n"));
  }

  TFile* f2 = TFile::Open(GForm("%s/ITS.Digits.root", data_dir));
  if(f2 == 0)
    throw(_eh + "can not open ITS.Digits.root.");
 
  const Text_t* ev_dir = GForm("Event%d", event);
  TDirectory* d = (TDirectory*)f2->Get(ev_dir);
  if(d == 0)
    throw(_eh + "can not get directory '"+ ev_dir +"'.");
  mTree = (TTree*)d->Get("TreeD");

  mDataDir = data_dir;
  mEvent   = event;

  SetITSSegmentation();
  
  // create tables for scaling
  // lowest scale factor refers unscaled ITS module
  mSPDScaleX[0]=1;
  mSPDScaleZ[0]=1;
  mSDDScaleX[0]=1;
  mSDDScaleZ[0]=1;
  mSSDScale[0]=1;
  // spd lows rsolution is in the level of 8x2 redaut chips
  Int_t nx = 8; // mSegSPD->Npx()/8; // 32
  Int_t nz = 6; // mSegSPD->Npz()/2; // 128

  mSPDScaleX[1] = Int_t(nx); 
  mSPDScaleZ[1] = Int_t(nz); 
  mSPDScaleX[2] = Int_t(nx*2); 
  mSPDScaleZ[2] = Int_t(nz*2); 
  mSPDScaleX[3] = Int_t(nx*3); 
  mSPDScaleZ[3] = Int_t(nz*3); 
  mSPDScaleX[4] = Int_t(nx*4); 
  mSPDScaleZ[4] = Int_t(nz*4); 


  mSDDScaleX[1] = 2;
  mSDDScaleZ[1] = 2;
  mSDDScaleX[2] = 8;
  mSDDScaleZ[2] = 8;
  mSDDScaleX[3] = 16;
  mSDDScaleZ[3] = 16;
  mSDDScaleX[4] = 25;
  mSDDScaleZ[4] = 25;

  mSSDScale[1] = 3;
  mSSDScale[2] = 9;
  mSSDScale[3] = 20;
  mSSDScale[4] = 30;

  
  // lowest scale factor refers unscaled ITS module
  mSPDScaleX[0]=1;
  mSPDScaleZ[0]=1;
  mSDDScaleX[0]=1;
  mSDDScaleZ[0]=1;
  mSSDScale[0]=1;
}

/**************************************************************************/

void ITSDigitsInfo::SetITSSegmentation()
{
  // SPD
  mSegSPD = new AliITSsegmentationSPD(mGeom);
  //SPD geometry  
  Int_t m;
  Float_t fNzSPD=160;
  Float_t fZ1pitchSPD=0.0425; Float_t fZ2pitchSPD=0.0625;
  Float_t fHlSPD=3.48;

  mSPDZCoord[0]=fZ1pitchSPD -fHlSPD;
  for (m=1; m<fNzSPD; m++) {
    Double_t dz=fZ1pitchSPD;
    if (m==31 || m==32 || m==63  || m==64  || m==95 || m==96 || 
        m==127 || m==128) dz=fZ2pitchSPD; 
    mSPDZCoord[m]=mSPDZCoord[m-1]+dz;
  }
  
  for (m=0; m<fNzSPD; m++) {
    Double_t dz=1.*fZ1pitchSPD;
    if (m==31 || m==32 || m==63  || m==64  || m==95 || m==96 || 
	m==127 || m==128) dz=1.*fZ2pitchSPD; 
    mSPDZCoord[m]-=dz;
  }
  
  // end of SPD geometry
  
  // SDD
  mSegSDD = new AliITSsegmentationSDD(mGeom);

  // SSD
  mSegSSD = new AliITSsegmentationSSD(mGeom);
}

void ITSDigitsInfo::GetSPDLocalZ(Int_t j, Float_t& z)
{
  z = mSPDZCoord[j];
}

/**************************************************************************/

TClonesArray* ITSDigitsInfo::GetDigits(Int_t mod, Int_t subdet)
{
  switch(subdet) {
  case 0: {
    TClonesArray* digitsSPD = 0;
    map<Int_t, TClonesArray*>::iterator i = mSPDmap.find(mod);
    if(i == mSPDmap.end()) {
      mTree->SetBranchAddress("ITSDigitsSPD",&digitsSPD);
      mTree->GetEntry(mod);
      mSPDmap[mod] = digitsSPD;
      return digitsSPD;
    } 
    else{
      return i->second;
    }
    break;
  }
  case 1: {
    TClonesArray* digitsSDD = 0;
    map<Int_t, TClonesArray*>::iterator i = mSDDmap.find(mod);
    if(i == mSDDmap.end()) {
      mTree->SetBranchAddress("ITSDigitsSDD",&digitsSDD);
      mTree->GetEntry(mod);
      mSDDmap[mod] = digitsSDD;
      return digitsSDD;
    } 
    else{
      return i->second;
    }
    break;
  }
  case 2: {
    TClonesArray* digitsSSD = 0;
    map<Int_t, TClonesArray*>::iterator i = mSSDmap.find(mod);
    if(i == mSSDmap.end()) {
      mTree->SetBranchAddress("ITSDigitsSSD",&digitsSSD);
      mTree->GetEntry(mod);
      mSSDmap[mod] = digitsSSD;
      return digitsSSD;
    } 
    else{
      return i->second;
    }
    break;
  }
  default:
    return 0;
  } //end switch
}


/**************************************************************************/

void ITSDigitsInfo::Dump()
{
  printf("*********************************************************\n");
  printf("SPD module dimension (%f,%f) \n",mSegSPD->Dx()*0.0001, mSegSPD->Dz()*0.0001);
  printf("SPD first,last module:: %d,%d \n", mGeom->GetStartSPD(),mGeom->GetLastSPD() );
  printf("SPD num cells per module (x::%d,z::%d)\n",mSegSPD->Npx(), mSegSPD->Npz());
  Int_t iz=0,ix = 0;
  printf("SPD dimesion of (%d,%d) in pixel(%f,%f) \n", ix,iz, mSegSPD->Dpx(ix), mSegSPD->Dpz(iz));
  iz = 32;
  printf("SPD dimesion of pixel (%d,%d) are (%f,%f) \n", ix,iz, mSegSPD->Dpx(ix)*0.001, mSegSPD->Dpz(iz)*0.001);
 
  printf("*********************************************************\n");
  printf("SDD module dimension (%f,%f) \n",mSegSDD->Dx()*0.0001, mSegSDD->Dz()*0.0001);
  printf("SDD first,last module:: %d,%d \n", mGeom->GetStartSDD(),mGeom->GetLastSDD() );
  printf("SDD num cells per module (x::%d,z::%d)\n",mSegSDD->Npx(), mSegSDD->Npz());
  printf("SDD dimesion of pixel are (%f,%f) \n", mSegSDD->Dpx(1)*0.001,mSegSDD->Dpz(1)*0.001);
  printf("*********************************************************\n");
  printf("SSD module dimension (%f,%f) \n",mSegSSD->Dx()*0.0001, mSegSSD->Dz()*0.0001);
  printf("SSD first,last module:: %d,%d \n", mGeom->GetStartSSD(),mGeom->GetLastSSD() );
  printf("SSD strips in module %d \n",mSegSSD->Npx());
  printf("SSD strip sizes are (%f,%f) \n", mSegSSD->Dpx(1),mSegSSD->Dpz(1));
  mSegSSD->SetLayer(5); Float_t ap,an;  mSegSSD->Angles(ap,an);
  printf("SSD layer 5 stereoP %f stereoN %f angle \n",ap,an); 
  mSegSSD->SetLayer(6);  mSegSSD->Angles(ap,an);
  printf("SSD layer 6 stereoP %f stereoN %f angle \n",ap,an); 
}


        /*
	  printf("num cells %d,%d scaled %d,%d \n",mSegSPD->Npz(),mSegSPD->Npx(),Nz,Nx);
	  printf("%d digits in ITSModule %d\n",ne, module);
	  Float_t zn = i*(3.48*2)/Nz - 3.48 ;
	  Float_t xo =  -mSegSPD->Dx()*0.00005 + mSegSPD->Dpx(0)*od->GetCoord2()*0.0001;
	  Float_t xn =  -mSegSPD->Dx()*0.00005 + j*0.0001*mSegSPD->Dx()/Nx;
	  Float_t dpx = 0.0001*mSegSPD->Dx()/Nx;
	  Float_t dpz = 3.48*2/Nz;
	  printf("Z::original (%3f) scaled (%3f, %3f) \n", zo, zn-dpz/2, zn+dpz/2);
	  printf("X::original (%3f) scaled (%3f, %3f) \n", xo, xn-dpx/2, xn+dpx/2);
	  printf("%d,%d maped to %d,%d \n", od->GetCoord1(), od->GetCoord2(), i,j );
	*/        
