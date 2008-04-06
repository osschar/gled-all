// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZMethodTagPreFilter_H
#define GledCore_ZMethodTagPreFilter_H

#include <Glasses/ZMirFilter.h>

class ZMethodTagPreFilter : public ZMirFilter {
  MAC_RNR_FRIENDS(ZMethodTagPreFilter);

private:
  void _init();

protected:
  TString	mTags;		// X{GS} 7 Textor(-width=>20)
  ZLink<ZMirFilter>	mFilter;	// X{gS} L{}

public:
  ZMethodTagPreFilter(const Text_t* n="ZMethodTagPreFilter", const Text_t* t=0) :
    ZMirFilter(n,t) { _init(); }

  virtual Result_e FilterMIR(ZMIR& mir);

#include "ZMethodTagPreFilter.h7"
  ClassDef(ZMethodTagPreFilter, 1)
}; // endclass ZMethodTagPreFilter


#endif
