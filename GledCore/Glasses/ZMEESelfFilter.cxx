// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZMEESelfFilter
//
//

#include "ZMEESelfFilter.h"

#include "ZMEESelfFilter.c7"

ClassImp(ZMEESelfFilter)

/**************************************************************************/

void ZMEESelfFilter::_init()
{
}

/**************************************************************************/

ZMirFilter::Result_e ZMEESelfFilter::FilterMIR(ZMIR& mir)
{
  // Allows execution if Alpha == Caller (which is a MEE) and
  // the method is tagged by the "MEE::Self" tag.
  // Otherwise returns a remapped R_None.

  const TString mee_self_tag("MEE::Self");

  if(mir.Caller == mir.Alpha) {
    GledNS::ClassInfo* ci = GledNS::FindClassInfo(FID_t(mir.Lid, mir.Cid));
    GledNS::MethodInfo* mi = ci->FindMethodInfo(mir.Mid);
    for(lStr_i i=mi->fTags.begin(); i!=mi->fTags.end(); ++i) {
      if(*i == mee_self_tag)
	return ZMirFilter::R_Allow;
    }
  }
  return PARENT_GLASS::FilterMIR(mir);
}
