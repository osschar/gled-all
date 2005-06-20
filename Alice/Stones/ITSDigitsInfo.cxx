// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ITSDigitsInfo
//
//

#include "ITSDigitsInfo.h"
#include <AliITSresponseSDD.h>

ClassImp(ITSDigitsInfo)

/**************************************************************************/

void ITSDigitsInfo::_init()
{
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
  delete mSegSPD; delete mSegSDD; delete mSegSSD; delete mGeom;
}

/**************************************************************************/
void ITSDigitsInfo::SetData(AliITSgeom* geom, TTree* tree)
{
  mGeom = geom;
  mTree = tree;
  SetITSSegmentation();
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
  AliITSresponseSDD *resp1=new AliITSresponseSDD("simulated");
  mSegSDD = new AliITSsegmentationSDD(mGeom, resp1);

  // SSD
  mSegSSD = new AliITSsegmentationSSD(mGeom);
}

void ITSDigitsInfo::GetSPDLocalZ(Int_t j, Float_t& z)
{
  z = mSPDZCoord[j];
}

/**************************************************************************/
TClonesArray*  ITSDigitsInfo::GetDigits(Int_t mod, Int_t subdet)
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
  }//end switch
}

/**************************************************************************/
void ITSDigitsInfo::Dump() {
  printf("*********************************************************\n");
  printf("SPD module dimension (%f,%f) \n",mSegSPD->Dx()*0.0001, mSegSPD->Dz()*0.0001);
  printf("SPD first,last module:: %d,%d \n", mGeom->GetStartSPD(),mGeom->GetLastSPD() );
  printf("SPD num cells per module (x::%d,z::%d)\n",mSegSPD->Npx(), mSegSPD->Npz());
  Int_t iz=0,ix = 0;
  printf("SPD dimesion of (%d,%d) in pixel(%f,%f) \n", ix,iz, mSegSPD->Dpx(ix), mSegSPD->Dpz(iz));
  iz = 32;
  printf("SPD dimesion of pixel (%d,%d) are (%f,%f) \n", ix,iz, mSegSPD->Dpx(ix)*0.001, mSegSPD->Dpz(iz)*0.001);
  /*
    Float_t x = 0.;
    printf("Cell dimension in x direction \n");
    for(Int_t l=0; l<mSegSPD->Npx(); l++) {
    x += mSegSPD->Dpx(l); 
    //printf("%d:%4f \n",l,mSegSPD->Dpx(l));
    }
    printf("\n");
    printf(">>> Summary in x direction %f \n",x*0.0001);

    Float_t z = 0.;
    printf("Cell dimension in z direction \n");
    for(Int_t l=0; l<mSegSPD->Npz(); l++) {
    z += mSegSPD->Dpz(l); // sum up to cell ix-1
    if(mSegSPD->Dpz(l)>mSegSPD->Dpz(0))
    printf("Z Dimension:: idx %d ==  %4f \n ",l,mSegSPD->Dpz(l));
    if(mSegSPD->Dpz(l)<mSegSPD->Dpz(0))
    printf("Z Dimension:: idx %d == %4f \n ",l,mSegSPD->Dpz(l));
    }
    printf("\n");
    printf(">>> Summary in z direction %f \n",z*0.0001);
  */
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
