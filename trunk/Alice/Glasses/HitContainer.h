// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_HitContainer_H
#define Alice_HitContainer_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

class HitContainer : public ZNode {
  MAC_RNR_FRIENDS(HitContainer);

private:
  void _init();

protected:
  Int_t       mNPoints;     //             X{GS} 7 ValOut()
  Int_t*      mPointLabels; //[mNPoints]   X{g}
  Float_t*    mPoints;      //[3*mNPoints] X{g}

  Float_t     mSize;        //             X{GS}  7 Value(-range=>[0,64,1,100])
  ZColor      mColor;       //             X{GSP} 7 ColorButt()

public:
  HitContainer(const Text_t* n="HitContainer", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~HitContainer();

  void Reset(Int_t n_points);
  void SetPoint(Int_t i, Int_t label, Float_t* pos);
  void SetPoint(Int_t i, Int_t label, Float_t x, Float_t y, Float_t z);
  void Print();

#include "HitContainer.h7"
  ClassDef(HitContainer, 1)
}; // endclass HitContainer


#endif
