// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_NestInfo_H
#define GledCore_NestInfo_H

#include <Glasses/SubShellInfo.h>
#include <Net/Ray.h>

class NestInfo : public SubShellInfo {
  MAC_RNR_FRIENDS(NestInfo);

public:
  enum LeafLayout_e {
    LL_Ants,
    LL_Custom
  };

  enum PrivRayQN_e  { PRQN_offset = RayNS::RQN_user_0,
		      PRQN_layout_change,
		      PRQN_rewidth
  };

private:
  void _init();

protected:
  // Creation parameters.
  Bool_t	bShowSelf;      // X{GS} 7 Bool(-join=>1)
  Int_t		mMaxChildExp;	// X{GS} 7 Value(-range=>[0,1000,1])

  // Widths of major elements.
  Int_t		mWName;   // X{GS} Ray{Rewidth} 7 Value(-range=>[18,50,1], -join=>1)
  Int_t		mWAnt;    // X{GS} Ray{Rewidth} 7 Value(-range=>[0,40,1])
  Int_t		mWIndent; // X{GS} Ray{Rewidth} 7 Value(-range=>[0,10,1], -join=>1)
  Int_t		mWSepBox; // X{GS} Ray{Rewidth} 7 Value(-range=>[0,5,1])

  // Layout specification.
  LeafLayout_e  mLeafLayout;    // X{GS} 7 PhonyEnum()
  TString       mLayout;        // X{GS} 7 Textor()

public:
  NestInfo(const Text_t* n="NestInfo", const Text_t* t=0) :
    SubShellInfo(n,t) { _init(); }

  void EnactLayout(); // X{E} 7 MButt()
  void ImportKings(); // X{E} 7 MButt()

  void EmitRewidthRay();

#include "NestInfo.h7"
  ClassDef(NestInfo, 1) // Glass representation of GUI browser 'FTW_Nest'.
}; // endclass NestInfo

GlassIODef(NestInfo);

#endif
