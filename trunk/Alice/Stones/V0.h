// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_V0_H
#define Alice_V0_H

#include <TObject.h>

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
  Int_t    fLabels[2];


  V0(const Text_t* n="V0", const Text_t* t=0) { _init(); }

#include "V0.h7"
  ClassDef(V0, 1)
}; // endclass V0

#endif
