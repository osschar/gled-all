// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GTS_GTSRetriangulator_H
#define GTS_GTSRetriangulator_H

#include <Glasses/ZGlass.h>
#include <Glasses/GTSurf.h>

class GTSRetriangulator : public ZGlass {
  MAC_RNR_FRIENDS(GTSRetriangulator);

public:
  enum StopOpts_e    { SO_Number,  SO_Cost };
  enum CostOpts_e    { CO_Length,  CO_Volume, CO_Angle };
  enum MidvertOpts_e { MO_Midvert, MO_Volume };
    
private:
  void _init();

protected:
  GTSurf*		mTarget;      // X{gS} L{}

  StopOpts_e		mStopOpts;    // X{gS} 7 PhonyEnum(-join=>1)
  UInt_t		mStopNumber;  // X{gS} 7 Value(-range=>[6,1e8,1,1])
  Double_t		mStopCost;    // X{gS} 7 Value(-range=>[0,1e9,1,1000])

  CostOpts_e		mCostOpts;        // X{gS} 7 PhonyEnum()
  Double_t		mVO_VolumeWght;   // X{gS} 7 Value(-join=>1, -range=>[0,1e9,1,1000])
  Double_t		mVO_BoundaryWght; // X{gS} 7 Value(-join=>1, -range=>[0,1e9,1,1000])
  Double_t		mVO_ShapeWght;    // X{gS} 7 Value(-range=>[0,1e9,1,100])

  MidvertOpts_e		mMidvertOpts; // X{gS} 7 PhonyEnum()

  Double_t		mMinAngleDeg; // X{gS} 7 Value(-range=>[0,180,1,1000])
public:
  GTSRetriangulator(const Text_t* n="GTSRetriangulator", const Text_t* t=0) : ZGlass(n,t) { _init(); }

  void Coarsen(); // X{ED} 7 MButt(-join=>1)
  void Refine();  // X{ED} 7 MButt()

#include "GTSRetriangulator.h7"
  ClassDef(GTSRetriangulator, 1) // Coarsen or refine GTSurf
}; // endclass GTSRetriangulator

GlassIODef(GTSRetriangulator);

#endif
