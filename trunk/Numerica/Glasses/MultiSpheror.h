// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Numerica_MultiSpheror_H
#define Numerica_MultiSpheror_H

#include <Glasses/ZHashList.h>

class MultiSpheror : public ZHashList {
  MAC_RNR_FRIENDS(MultiSpheror);

private:
  void _init();

protected:
  UInt_t	mNVert;		// X{GS} 7 Value(-range=>[2,100,1,1], -join=>1)
  Float_t	mDx;		// X{GS} 7 Value(-range=>[1,20,1,10])
  UShort_t	mNGrid;		// X{GS} 7 Value(-range=>[1,20,1],-join=>1)

private:
  UShort_t	mNG;		// X{G}  7 ValOut(-range=>[0,20,1])
  UShort_t	mNtoDo;		// X{G}  7 ValOut(-range=>[0,400,1],-join=>1)
  UShort_t	mNDone;		// X{G}  7 ValOut(-range=>[0,400,1])

public:
  MultiSpheror(const Text_t* n="MultiSpheror", const Text_t* t=0) :
    ZHashList(n,t) { _init(); }

  void Init();	    // X{E} C{0} 7 MButt(-join=>1)
  void ClaimCPUs(); // X{E} C{0} 7 MButt()

#include "MultiSpheror.h7"
  ClassDef(MultiSpheror, 1)
}; // endclass MultiSpheror


#endif
