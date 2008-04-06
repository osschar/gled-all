// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Audio1_AlSource_H
#define Audio1_AlSource_H

#include <Glasses/ZNode.h>

class AlSource : public ZNode {
  MAC_RNR_FRIENDS(AlSource);

private:
  void _init();

protected:
  TString     mFile;    // X{GS} 7 Filor()

  Float_t     mGain;    // X{GS} Ray{Source} 7 Value(-range=>[0,100,1,1000])
  Float_t     mMinGain; // X{GS} Ray{Source} 7 Value(-range=>[0,100,1,1000], -join=>1)
  Float_t     mMaxGain; // X{GS} Ray{Source} 7 Value(-range=>[0,100,1,1000])
  Float_t     mPitch;   // X{GS} Ray{Source} 7 Value(-range=>[0.001,1,1,1000])

  Float_t     mConeInnerAngle; // X{GS} Ray{Cone} 7 Value(-range=>[0,360,1,1000])
  Float_t     mConeOuterAngle; // X{GS} Ray{Cone} 7 Value(-range=>[0,360,1,1000])
  Float_t     mConeOuterGain;  // X{GS} Ray{Cone} 7 Value(-range=>[0,1,  1,1000])

  UInt_t      mAlBuf;	//!
  UInt_t      mAlSrc;	//!

public:
  AlSource(const Text_t* n="AlSource", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~AlSource();

  void Play(Int_t count=1); // X{Ed} 7 MCWButt()

  void EmitSourceRay();
  void EmitConeRay();

#include "AlSource.h7"
  ClassDef(AlSource, 1)
}; // endclass AlSource


#endif
