// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZIdentityListFilter
//
//

#include "ZIdentityListFilter.h"

#include "ZIdentityListFilter.c7"

ClassImp(ZIdentityListFilter)

/**************************************************************************/

void ZIdentityListFilter::_init()
{
  // !!!! Set all links to 0 !!!!
  mIdentities = 0;
  mOnMatch = ZMirFilter::R_Allow;
}

/**************************************************************************/

ZMirFilter::Result_e ZIdentityListFilter::FilterMIR(ZMIR& mir)
{
  if(mIdentities) {
    lpZGlass_i i, end;
    mIdentities->BeginIteration(i, end);
    while(i != end) {
      ZIdentity *id = (ZIdentity*)*i;
      if(mir.Caller->HasIdentity(id)) {
	mIdentities->EndIteration();
	return (Result_e)mOnMatch;
      }
      ++i;
    }
    mIdentities->EndIteration();
    return NegateResult((Result_e)mOnMatch);
  }
  return PARENT_GLASS::FilterMIR(mir);
}

/**************************************************************************/
