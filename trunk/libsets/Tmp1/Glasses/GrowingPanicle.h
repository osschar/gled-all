// $Id: Glass_SKEL.h 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_GrowingPanicle_H
#define Tmp1_GrowingPanicle_H

#include <Glasses/GrowingPlant.h>

class GrowingPanicle : public GrowingPlant
{
  MAC_RNR_FRIENDS(GrowingPanicle);

private:
  void _init();

protected:
  virtual void SegmentStepTime(Segments_i ref, Segments_t& in, Segments_t& out);  
  
  float mLateralAngle;          // X{GST} 7 Value(-range=>[0, 90, 1])
  
  ZColor		mSColor;  // X{PGST} 7 ColorButt(-join=>1)
  float     mSSize;  // X{GST} 7 Value(-range=>[0, 1, 1, 100])
  
  ZColor		mTColor;  // X{PGST} 7 ColorButt(-join=>1)
  float     mTSize;  // X{GST} 7 Value(-range=>[0, 1, 1, 100])
  
  ZColor		mUColor;  // X{PGST} 7 ColorButt(-join=>1)
  float     mUSize;  // X{GST} 7 Value(-range=>[0, 1, 1, 100])
  
public:
  GrowingPanicle(const Text_t* n="GrowingPanicle", const Text_t* t=0);
  virtual ~GrowingPanicle();

#include "GrowingPanicle.h7"
  ClassDef(GrowingPanicle, 1);
}; // endclass GrowingPanicle

#endif
