// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ZMethodTagPreFilter
//
// Contains a comma-separated list of tags and a link to ZMirFilter.
// 

#include "ZMethodTagPreFilter.h"

#include "ZMethodTagPreFilter.c7"

ClassImp(ZMethodTagPreFilter)

/**************************************************************************/

void ZMethodTagPreFilter::_init()
{
  // !!!! Set all links to 0 !!!!
  mFilter = 0;
}

/**************************************************************************/
ZMirFilter::Result_e ZMethodTagPreFilter::FilterMIR(ZMIR& mir)
{
  // If Method being called contains one of the tags in mTags, then
  // the result of mFilter->FilterMIR(mir) is returned.
  //
  // This is written in a terrribly inefficient manner.

  lStr_t tags;
  int cnt = GledNS::split_string(mTags.Data(), tags, ',');
  if(cnt) {
    GledNS::ClassInfo* ci = GledNS::FindClassInfo(FID_t(mir.Lid, mir.Cid));
    GledNS::MethodInfo* mi = ci->FindMethodInfo(mir.Mid);
    for(lStr_i i=tags.begin(); i!=tags.end(); ++i) {
      for(lStr_i j=mi->fTags.begin(); j!=mi->fTags.end(); ++j) {
	if(*i == *j) {
	  if(mFilter != 0)
	    return mFilter->FilterMIR(mir);
	  else
	    return PARENT_GLASS::FilterMIR(mir);
	}
      }
    }
  }

  return R_None;
}

/**************************************************************************/
