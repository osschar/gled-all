// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_MetaSubViewInfo_H
#define GledCore_MetaSubViewInfo_H

#include <Glasses/ZList.h>

class MetaSubViewInfo : public ZList {
  MAC_RNR_FRIENDS(MetaSubViewInfo);

private:
  void _init();

protected:
  Int_t		mX;      // X{GS} 7 Value(-range=>[0,256,1], -join=>1)
  Int_t		mY;      // X{GS} 7 Value(-range=>[0,256,1])

public:
  MetaSubViewInfo(const Text_t* n="MetaSubViewInfo", const Text_t* t=0) :
    ZList(n,t) { _init(); }


#include "MetaSubViewInfo.h7"
  ClassDef(MetaSubViewInfo, 1)
}; // endclass MetaSubViewInfo

GlassIODef(MetaSubViewInfo);

#endif
