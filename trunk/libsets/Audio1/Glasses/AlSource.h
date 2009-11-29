// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Audio1_AlSource_H
#define Audio1_AlSource_H

#include <Glasses/ZNode.h>

class AlBuffer;

class AlSource : public ZNode
{
  MAC_RNR_FRIENDS(AlSource);

private:
  void _init();

protected:
  ZLink<AlBuffer>  mDefaultBuffer; // X{GS} L{}

  Float_t     mGain;    // X{GS} Ray{Source} 7 Value(-range=>[0,100,1,1000])
  Float_t     mMinGain; // X{GS} Ray{Source} 7 Value(-range=>[0,100,1,1000], -join=>1)
  Float_t     mMaxGain; // X{GS} Ray{Source} 7 Value(-range=>[0,100,1,1000])
  Float_t     mPitch;   // X{GS} Ray{Source} 7 Value(-range=>[0.001,1,1,1000])

  Float_t     mConeInnerAngle; // X{GS} Ray{Cone} 7 Value(-range=>[0,360,1,1000])
  Float_t     mConeOuterAngle; // X{GS} Ray{Cone} 7 Value(-range=>[0,360,1,1000])
  Float_t     mConeOuterGain;  // X{GS} Ray{Cone} 7 Value(-range=>[0,1,  1,1000])

  UInt_t      mAlSrc;	//!

public:
  AlSource(const Text_t* n="AlSource", const Text_t* t=0) :
    ZNode(n,t) { _init(); }
  virtual ~AlSource();

  Bool_t IsPlaying();

  void QueueBuffer(AlBuffer* buf=0, Int_t count=1); // X{E} C{1} 7 MCWButt()

  void Play();     // X{E} 7 MCWButt(-join=>1)
  void Loop();     // X{E} 7 MCWButt(-join=>1)
  void Stop();     // X{E} 7 MCWButt()

  void EmitSourceRay();
  void EmitConeRay();

  void PrintSourceInfo();  //! X{E} 7 MButt()

#include "AlSource.h7"
  ClassDef(AlSource, 1);
}; // endclass AlSource


#endif
