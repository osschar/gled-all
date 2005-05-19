// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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

  Float_t     mGain;    // X{GS} & Value(-range=>[0,100,1,100])

  UInt_t      mAlBuf;	//!
  UInt_t      mAlSrc;	//!

public:
  AlSource(const Text_t* n="AlSource", const Text_t* t=0) :
    ZNode(n,t) { _init(); }

  void Play(Int_t count=1); // X{Ed} 7 MCWButt()

  //void SetGain() {

#include "AlSource.h7"
  ClassDef(AlSource, 1)
}; // endclass AlSource

GlassIODef(AlSource);

#endif
