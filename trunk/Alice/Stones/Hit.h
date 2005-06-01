// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_Hit_H
#define Alice_Hit_H

#include <TObject.h>
#include <TMath.h>

class Hit : public TObject {
 public:
  Float_t  x,y,z;
  UChar_t  fDetID;   // X{GS} 
  Int_t    fLabel;   // X{GS} 
  Int_t    fEvaLabel;   // X{GS} 

  Hit() : TObject() {}
  Hit(UChar_t detector, Int_t particle, Int_t eva,
      Float_t x, Float_t y, Float_t z );

  /**************************************************************************/
  // methods needed for combined tree selection
  Double_t  R()            const { return TMath::Sqrt(x*x+y*y);}

  Double_t  P()            const { return TMath::Sqrt(x*x+y*y+z*z);}
  Double_t  Pt()           const { return TMath::Sqrt(x*x+y*y);}
  Double_t  Eta()          const { 
    if (TMath::Abs(R() != z)) return 0.5*TMath::Log((R()+z)/(R()-z));
    else  return 1.e30;}

  Double_t  Theta()        const { return (z==0)?TMath::PiOver2():TMath::ACos(z/R());}
  
  /**************************************************************************/
  void     Dump();
#include "Hit.h7"
  ClassDef(Hit, 1)
    }; // endclass Hit

#endif
