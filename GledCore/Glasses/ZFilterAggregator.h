// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZFilterAggregator_H
#define GledCore_ZFilterAggregator_H

#include <Glasses/ZMirFilter.h>
#include <Glasses/ZHashList.h>

class ZFilterAggregator : public ZMirFilter {
  MAC_RNR_FRIENDS(ZFilterAggregator);

private:
  void _init();

protected:
  Bool_t	bStrongNone;	// X{gS} 7 Bool(-join=>1)
  Bool_t	bPreemptNone;	// X{gS} 7 Bool()
  Bool_t	bPreemptAllow;	// X{gS} 7 Bool(-join=>1)
  Bool_t	bPreemptDeny;	// X{gS} 7 Bool()

  ZHashList*	mFilters;	// X{gS} L{}

public:
  ZFilterAggregator(const Text_t* n="ZFilterAggregator", const Text_t* t=0) :
    ZMirFilter(n,t) { _init(); }

  UChar_t BuildPreemptionBits();

  virtual Result_e FilterMIR(ZMIR& mir);

#include "ZFilterAggregator.h7"
  ClassDef(ZFilterAggregator, 1)
}; // endclass ZFilterAggregator

GlassIODef(ZFilterAggregator);

/**************************************************************************/
// Inlines
/**************************************************************************/

inline
UChar_t ZFilterAggregator::BuildPreemptionBits()
{
  UChar_t res = 0;
  if(bPreemptNone)  res |= ZMirFilter::R_None;
  if(bPreemptAllow) res |= ZMirFilter::R_Allow;
  if(bPreemptDeny)  res |= ZMirFilter::R_Deny;
  return res;
}

#endif
