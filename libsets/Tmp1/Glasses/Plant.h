// $Id: Glass_SKEL.h 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

/**************************************************************************/
// Plant - extension of Weed class. Has virtual functions to draw
// segments, leaf and flower as polygons.
/**************************************************************************/

#ifndef Tmp1_Plant_H
#define Tmp1_Plant_H

#include <Glasses/Weed.h>

class Plant : public Weed
{
  MAC_RNR_FRIENDS(Plant);
  
private:
  void _init();
  
protected:
  
public:
  Plant(const Text_t* n="Plant", const Text_t* t=0);
  virtual ~Plant();
  
  Double_t mStemWidth;   // X{GST} 7 Value(-range=>[0, 1, 1, 1000])
  Double_t mLeafSize;    // X{GST} 7 Value(-range=>[0, 1, 1, 100])
  Double_t mFlowerSize;  // X{GST} 7 Value(-range=>[0, 1, 1, 100])
  
#include "Plant.h7"
  ClassDef(Plant, 1);
}; // endclass Plant

#endif
