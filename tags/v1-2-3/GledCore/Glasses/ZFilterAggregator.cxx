// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZFilterAggregator
//
//

#include "ZFilterAggregator.h"

#include "ZFilterAggregator.c7"

ClassImp(ZFilterAggregator)

/**************************************************************************/

void ZFilterAggregator::_init()
{
  // !!!! Set all links to 0 !!!!
  bStrongNone = bPreemptNone = false;
  bPreemptAllow = true;
  bPreemptDeny  = false;
  mFilters = 0;
}

/**************************************************************************/

ZMirFilter::Result_e ZFilterAggregator::FilterMIR(ZMIR& mir)
{
  if(mFilters) {
    lpZGlass_i i, end;
    UChar_t result  = 0;
    UChar_t preempt = BuildPreemptionBits();
    mFilters->BeginIteration(i, end);
    while(i != end) {
      UChar_t r = ((ZMirFilter*)*i)->FilterMIR(mir);
      if(r & preempt) {
	mFilters->EndIteration();
	return FinaliseResult((Result_e)r);
      }
      result |= r;
      if(result & R_Allow && result & R_Deny) break;
      ++i;
    }
    mFilters->EndIteration();
    if( (result & R_Allow && result & R_Deny) ||
        (result & ZMirFilter::R_None && bStrongNone) )
      {    
	return FinaliseResult(R_None);
      }
    if(result & R_Allow) return R_Allow;
    if(result & R_Deny)  return R_Deny;
  }
  return PARENT_GLASS::FilterMIR(mir);
}


/**************************************************************************/
