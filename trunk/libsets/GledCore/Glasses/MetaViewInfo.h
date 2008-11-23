// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_MetaViewInfo_H
#define GledCore_MetaViewInfo_H

#include <Glasses/ZList.h>

class MetaViewInfo : public ZList {
  MAC_RNR_FRIENDS(MetaViewInfo);

private:
  void _init();

protected:
  Int_t		mW;        // X{GS} 7 Value(-range=>[1,256,1], -join=>1)
  Int_t		mH;        // X{GS} 7 Value(-range=>[1,256,1])

  Bool_t	bExpertP;  // X{GS} 7 Bool()

public:
  MetaViewInfo(const Text_t* n="MetaViewInfo", const Text_t* t=0) :
    ZList(n,t) { _init(); }

  void Size(int w, int h);  // X{E}

#include "MetaViewInfo.h7"
  ClassDef(MetaViewInfo, 1);
}; // endclass MetaViewInfo


#endif
