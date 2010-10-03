// $Id: Glass_SKEL.h 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_MonopodialHerb_H
#define Tmp1_MonopodialHerb_H

#include <Glasses/GrowingPlant.h>

class MonopodialHerb : public GrowingPlant
{
  MAC_RNR_FRIENDS(MonopodialHerb);

private:
  void _init();

protected:
  virtual void SegmentStepTime(Segments_i ref, Segments_t& out);  
  
public:
  MonopodialHerb(const Text_t* n="MonopodialHerb", const Text_t* t=0);
  virtual ~MonopodialHerb();

#include "MonopodialHerb.h7"
  ClassDef(MonopodialHerb, 1);
}; // endclass MonopodialHerb

#endif
