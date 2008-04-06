// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_GForger_H
#define Geom1_GForger_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZImage.h>
#include <Glasses/RectTerrain.h>

class GForger : public ZGlass
{
  MAC_RNR_FRIENDS(GForger);

private:
  void _init();

protected:
  ZLink<ZImage>       mImage;   // X{GS} L{}
  ZLink<RectTerrain>  mTerrain; // X{GS} L{}
  Float_t             mZFactor; // X{GS} 7 Value(-range=>[0,1000,1,1000])

  TString mFileName;  // X{GS} 7 Textor()

  Int_t   mMesh;      // X{GS} 7 Value(-range=>[0,4096,1], -join=>1)
  Float_t mPower;     // X{GS} 7 Value(-range=>[0,16,1,1000])

  Float_t mDimension; // X{GS} 7 Value(-range=>[0,4,1,1000])
  TString mADimText;  // X{GS} 7 Textor()

  Bool_t  bCraters;       // X{GS} 7 Bool(-join=>1)
  Float_t mCraterDensity; // X{GS} 7 Value(-range=>[0,1000,1,100], join=>1)
  Float_t mCraterHeight;  // X{GS} 7 Value(-range=>[0,1000,1,100])

public:
  GForger(const Text_t* n="GForger", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  void Forge(); // X{ED} 7 MButt()

#include "GForger.h7"
  ClassDef(GForger, 1)
}; // endclass GForger


#endif
