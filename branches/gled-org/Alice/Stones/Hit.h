// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_Hit_H
#define Alice_Hit_H

#include <TObject.h>

class Hit : public TObject {

 public:
  Hit() : TObject() {}
  Hit(UChar_t detector, Int_t particle, Int_t eva,
      Float_t x, Float_t y, Float_t z );

  void     Dump();

  Float_t  x,y,z;
  UChar_t  det_ID;
  Int_t    particle_ID;
  Int_t    eva_ID;
  

#include "Hit.h7"
  ClassDef(Hit, 1)
}; // endclass Hit

#endif
