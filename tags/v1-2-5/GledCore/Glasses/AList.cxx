// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AList
//
//

#include "AList.h"

#include "AList.c7"

ClassImp(AList::ElRep)

ClassImp(AList)

/**************************************************************************/

void AList::_init()
{
  // !!!! Set all links to 0 !!!!
}

/**************************************************************************/

void AList::new_element_check(ZGlass* g)
{
  if(g == 0) {
    throw(string("ZList::new_element_check called with null ZGlass*"));
  }
  if(mLid && mCid) {
    if(!GledNS::IsA(g, FID_t(mLid, mCid))) {
      throw(string("AList::new_element_check lens of wrong FID_t"));
    }
  }
}

/**************************************************************************/

Int_t AList::RebuildAllRefs(An_ID_Demangler* idd)
{
  return RebuildLinkRefs(idd) + RebuildListRefs(idd);
}

/**************************************************************************/

void AList::Streamer(TBuffer &b)
{
  static const string _eh("AList::Streamer ");
  UInt_t R__s, R__c;

  if(b.IsReading()) {

    Version_t v = b.ReadVersion(&R__s, &R__c);
    ZGlass::Streamer(b);
    b >> mSize >> mLid >> mCid;
    ISdebug(D_STREAM, GForm("%sreading %d elements (%d,%d).",
			    _eh.c_str(), mSize, mLid, mCid));
   b.CheckByteCount(R__s, R__c, AList::IsA());

  } else {

    R__c = b.WriteVersion(AList::IsA(), kTRUE);
    ZGlass::Streamer(b);
    b << mSize << mLid << mCid;
    ISdebug(D_STREAM, GForm("%swriting %d elements (%d,%d).",
			    _eh.c_str(), mSize, mLid, mCid));
    b.SetByteCount(R__c, kTRUE);

  }
}

/**************************************************************************/
