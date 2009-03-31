// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZIdentityFilter
//
//

#include "ZIdentityFilter.h"
#include "ZIdentityFilter.c7"

ClassImp(ZIdentityFilter);

/**************************************************************************/

void ZIdentityFilter::_init()
{
  mIdentity = 0;
  mOnMatch = ZMirFilter::R_Allow;
}

/**************************************************************************/

ZMirFilter::Result_e ZIdentityFilter::FilterMIR(ZMIR& mir)
{
  if(mIdentity != 0) {
    if(mir.fCaller->HasIdentity(mIdentity.get()))
      return (Result_e)mOnMatch;
    else
      return NegateResult((Result_e)mOnMatch);
  }
  return PARENT_GLASS::FilterMIR(mir);
}


/**************************************************************************/
