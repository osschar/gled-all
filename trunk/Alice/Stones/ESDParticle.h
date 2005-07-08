// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ESDParticle_H
#define Alice_ESDParticle_H

#include <TObject.h>
#include <TMath.h>

class ESDParticle : public TObject
{
 private:
  void _init();

 public: 
  Int_t    fSign;   // X{G}
  Int_t    fLabel;  // X{G}
  Int_t    fStatus; // X{G}
  Double_t fV[3];   // birth point
  Double_t fP[3];   // momentum

  ESDParticle(const Text_t* n="ESDParticle", const Text_t* t=0) :
    TObject()
  { _init(); }

  ESDParticle (Double_t* v, Double_t* p, Int_t lab, Int_t sign);
  
  /**************************************************************************/
  // methods needed for combined tree selection

  Double_t  R()            const { return TMath::Sqrt(fV[0]*fV[0]+fV[1]*fV[1]);}

  Double_t  P()            const { return TMath::Sqrt(fP[0]*fP[0]+fP[1]*fP[1]+fP[2]*fP[2]); }
  Double_t  Pt()           const { return TMath::Sqrt(fP[0]*fP[0]+fP[1]*fP[1]);}
  Double_t  Eta()          const { 
    if (TMath::Abs(P() != fP[2])) return 0.5*TMath::Log((P()+fP[2])/(P()-fP[2]));
    else  return 1.e30;
  }
  Double_t  Theta()        const { return (fP[2]==0)?TMath::PiOver2():TMath::ACos(fP[2]/P()); }
  Double_t  Phi()          const { return TMath::ATan2(fP[1], fP[0]); }

  /**************************************************************************/

#include "ESDParticle.h7"
  ClassDef(ESDParticle, 1)
}; // endclass ESDParticle

#endif
