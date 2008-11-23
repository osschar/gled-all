// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZIdentityListFilter
//
//

#include "ZIdentityListFilter.h"

#include "ZIdentityListFilter.c7"

ClassImp(ZIdentityListFilter);

/**************************************************************************/

void ZIdentityListFilter::_init()
{
  mOnMatch = ZMirFilter::R_Allow;
}

/**************************************************************************/

ZMirFilter::Result_e ZIdentityListFilter::FilterMIR(ZMIR& mir)
{
  if(mIdentities != 0) {
    GMutexHolder ids_lock(mIdentities->RefListMutex());
    AList::Stepper<> s(*mIdentities);
    while(s.step()) {
      if(mir.fCaller->HasIdentity((ZIdentity*)*s))
	return (Result_e)mOnMatch;
    }
    return NegateResult((Result_e)mOnMatch);
  }
  return PARENT_GLASS::FilterMIR(mir);
}

/**************************************************************************/
