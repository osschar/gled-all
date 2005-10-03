// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_ZRibbon_H
#define Geom1_ZRibbon_H

#include <Glasses/ZGlass.h>
#include <Stones/ZColorMark.h>

class ZRibbon : public ZGlass {
  MAC_RNR_FRIENDS(ZRibbon);

private:
  void _init();

protected:
  TString	mPOVFile;	   //  X{GS}  7 Filor()
  Bool_t	bLoadAdEnlight;    //  X{GS}  7 Bool()

  list<ZColorMark> mColMarks;	   //  X{R}

public:
  ZRibbon(const Text_t* n="ZRibbon", const Text_t* t=0) : ZGlass(n,t) { _init(); }

  ZColor MarkToCol(Float_t mark);

  void LoadPOV(); // X{E} 7 MButt()

#include "ZRibbon.h7"
  ClassDef(ZRibbon, 1)
}; // endclass ZRibbon


#endif
