// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_V0_H
#define Alice_V0_H

#include <TObject.h>
#include <TMath.h>

class V0 : public TObject {

 private:
  void _init();

 protected:
   
 public:
  Int_t    fStatus;

  // vertices of negative and positive
  Double_t fVM[3];
  Double_t fVP[3];

  // distance to closest approach
  Double_t fDCA[3];

  // birth point of neutral, mother particle
  Double_t fV0[3];

  // mometum of the charged particle
  Double_t fPM[3];
  Double_t fPP[3];

  // data from simulation ???
  Int_t    fPDG;
  Int_t    fDLabels[2];
  Int_t    fLabel;  // mother label read from kinematics


  V0(const Text_t* n="V0", const Text_t* t=0) { _init(); }
  Double_t  R() const {return TMath::Sqrt(fDCA[0]*fDCA[0]+fDCA[1]*fDCA[1]);}

  
   
  Double_t  P()  const {return TMath::Sqrt((fPM[0]+fPP[0])*(fPM[0]+fPP[0])+
					   (fPM[1]+fPP[1])*(fPM[1]+fPP[1])+
					   (fPM[2]+fPP[2])*(fPM[2]+fPP[2]));}    

  Double_t  Pt()  const {return TMath::Sqrt((fPM[0]+fPP[0])*(fPM[0]+fPP[0])+
					    (fPM[1]+fPP[1])*(fPM[1]+fPP[1]));}    

  Double_t  Theta() const { Double_t pz =  fPM[3]+fPP[3];
    return (pz==0)?TMath::PiOver2():TMath::ACos(pz/P()); }

  Double_t  Eta()   const {  Double_t pz =  fPM[3]+fPP[3];
    if (TMath::Abs(P() != pz)) return 0.5*TMath::Log((P()+pz)/(P()-pz));
    else return 1.e30;
  }


#include "V0.h7"
  ClassDef(V0, 1)
    }; // endclass V0

#endif
