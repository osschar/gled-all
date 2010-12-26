// $Id: Glass_SKEL.h 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_DibotryoidHerb_H
#define Tmp1_DibotryoidHerb_H

#include <Glasses/GrowingPlant.h>

class DibotryoidHerb : public GrowingPlant
{
  MAC_RNR_FRIENDS(DibotryoidHerb);

private:
  void _init();
  GrowingPlant::Segments_i  NeighbourBack(Segments_i startIt);

protected:
  virtual void SegmentStepTime(Segments_i ref, Segments_t& out);  
  
  int mSendSignalDelay;        // X{GST} 7 Value(-range=>[1, 20, 1])
  int mCreateBranchDelay;      // X{GST} 7 Value(-range=>[1, 10, 1])
  int mPlastocronMainAxis;     // X{GST} 7 Value(-range=>[1, 10, 1])
  int mPlastocronLateralAxis;  // X{GST} 7 Value(-range=>[1, 10, 1])
  int mSignalDelayMainAxis;    // X{GST} 7 Value(-range=>[1, 10, 1])
  int mSignalDelayLateralAxis; // X{GST} 7 Value(-range=>[1, 10, 1])
 
  float mLateralAngle;          // X{GST} 7 Value(-range=>[0, 360, 1])
  
public:
  DibotryoidHerb(const Text_t* n="DibotryoidHerb", const Text_t* t=0);
  virtual ~DibotryoidHerb();

#include "DibotryoidHerb.h7"
  ClassDef(DibotryoidHerb, 1);
}; // endclass DibotryoidHerb

#endif
