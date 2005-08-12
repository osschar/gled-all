// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_ITSModule_H
#define Alice_ITSModule_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
#include <Stones/ITSDigitsInfo.h>

class ITSModule : public ZNode {
  MAC_RNR_FRIENDS(ITSModule);
  MAC_RNR_FRIENDS(ITSScaledModule);

 private:
  void _init();

 protected:
  virtual void init_module();
  virtual void set_trans();

  ITSDigitsInfo* mInfo;

  Bool_t      bSetTrans; // X{GS}  7 Bool()
  Int_t       mID;       // X{g}   7 Value(-range=>[0,2200,1], -join=>1)
  Int_t       mDetID;    // X{G}   7 ValOut()

  Int_t       mLayer;    // X{gS}  7 ValOut(-range=>[0,6,1], -join=>1)
  Int_t       mLadder;   // X{gS}  7 ValOut(-range=>[0,100,1], -join=>1)
  Int_t       mDet;      // X{gS}  7 ValOut(-range=>[0,100,1])
  
  Float_t     mDx;       // X{G}  7 ValOut(-join=>1)
  Float_t     mDz;       // X{G}  7 ValOut(-join=>1)
  Float_t     mDy;       // X{G}  7 ValOut()

 public:
  ITSModule(const Text_t* n="ITSModule", const Text_t* t=0) : ZNode(n,t)
  { _init(); }
  ITSModule(Int_t id, ITSDigitsInfo* info);
  virtual ~ITSModule();

  virtual void SetID(Int_t id); // X{E}

#include "ITSModule.h7"
  ClassDef(ITSModule, 1)
    }; // endclass ITSModule

GlassIODef(ITSModule);

#endif
