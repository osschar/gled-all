// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZIdentityFilter
//
//

#include "ZIdentityFilter.h"

#include "ZIdentityFilter.c7"

ClassImp(ZIdentityFilter)

/**************************************************************************/

void ZIdentityFilter::_init()
{
  // !!!! Set all links to 0 !!!!
  mIdentity = 0;
  mOnMatch = ZMirFilter::R_Allow;
}

/**************************************************************************/

ZMirFilter::Result_e ZIdentityFilter::FilterMIR(ZMIR& mir)
{
  if(mIdentity) {
    if(mir.Caller->HasIdentity(mIdentity))
      return (Result_e)mOnMatch;
    else
      return NegateResult((Result_e)mOnMatch);
  }
  return PARENT_GLASS::FilterMIR(mir);
}


/**************************************************************************/
