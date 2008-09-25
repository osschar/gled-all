// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_HitContainer_H
#define Alice_HitContainer_H

#include <Glasses/ZNode.h>
#include <Stones/Hit.h>
#include <Stones/ZColor.h>

class HitContainer : public ZNode {
  MAC_RNR_FRIENDS(HitContainer);

public:
  typedef vector<Hit*>                  vHit_t;
  typedef vector<Hit*>::iterator        vHit_i;
  typedef vector<Hit*>::const_iterator  vHit_ci;

private:
  void _init();

protected:
  vHit_t                   mHits;          // X{R} 
  ZColor                   mColor;         // X{GSP} 7 ColorButt()
  Int_t                    mNHits;         // X{GS}  7 ValOut()
  Int_t         	   mEvaLabel;      // X{GS}    

public:
  HitContainer(const Text_t* n="HitContainer", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~HitContainer();

  void AddHit(Hit* ah) {mHits.push_back(ah); mNHits++;}
  void Clear();				   // X{E}  7 MButt()
  void Dump();				   // X{E}  7 MButt()	

#include "HitContainer.h7"
  ClassDef(HitContainer, 1)
}; // endclass HitContainer

GlassIODef(HitContainer);

#endif
