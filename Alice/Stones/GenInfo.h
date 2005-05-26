// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_GenInfo_H
#define Alice_GenInfo_H

#include <Stones/MCParticle.h>
#include <Stones/ESDTrack.h>

#include <TObject.h>

class GenInfo : public TObject {

 private:
  void _init();

 protected:

 public:
  MCParticle   P;   // kinematics
  ESDTrack     R;   // reconstruction
  Bool_t       bR;  // is reconstructed
  Int_t        Nh;  // number of hits
  Int_t        Nc;  // number of clusters

  GenInfo(const Text_t* n="GenInfo", const Text_t* t=0) :
    TObject()
  { _init(); }
  //virtual ~GenInfo() { delete P; delete R; }

#include "GenInfo.h7"
  ClassDef(GenInfo, 1)
    }; // endclass GenInfo

#endif
