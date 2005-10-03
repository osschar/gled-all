// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SubShellInfo_H
#define GledCore_SubShellInfo_H

#include <Glasses/ZList.h>

class SubShellInfo : public ZList {
  MAC_RNR_FRIENDS(SubShellInfo);

private:
  void _init();

protected:
  TString	mCtorLibset;	// X{GS} 7 Textor()
  TString	mCtorName;	// X{GS} 7 Textor()

  Int_t		mDefW;          // X{GS} 7 Value(-range=>[1,256,1], -join=>1)
  Int_t		mDefH;          // X{GS} 7 Value(-range=>[1,256,1])

public:
  SubShellInfo(const Text_t* n="SubShellInfo", const Text_t* t=0) :
    ZList(n,t) { _init(); }


#include "SubShellInfo.h7"
  ClassDef(SubShellInfo, 1)
}; // endclass SubShellInfo


#endif
