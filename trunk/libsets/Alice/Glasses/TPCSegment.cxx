// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TPCDigit
//
//

#include "TPCSegment.h"
#include "TPCSegment.c7"

ClassImp(TPCSegment)

/**************************************************************************/

void TPCSegment::_init()
{
  // Override settings from ZGlass
  bUseDispList = true;

  mSegment = 0;
  mDigInfo = 0;
}

TPCSegment::~TPCSegment()
{
  if(mDigInfo) mDigInfo->DecRefCount();
}

/**************************************************************************/

void TPCSegment::SetDigInfo(TPCDigitsInfo* diginfo)
{
  if(mDigInfo) mDigInfo->DecRefCount();
  mDigInfo = diginfo;
  if(mDigInfo) mDigInfo->IncRefCount();
  mStampReqTring = Stamp(FID());
}

void TPCSegment::SetSegment(Int_t segment)
{
  if(segment < 0 ) segment = 0;
  if(segment > 36) segment = 36;
  mSegment = segment;
  SetName(GForm("TPCSegment %d", mSegment));
  mStampReqTring = Stamp(FID());
}
