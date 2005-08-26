// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// WGlValuator
//
// In principle should install Alpha-Observer ... but pupils are ROARs sofar.

#include "WGlValuator.h"
#include "WGlValuator.c7"

#include <TClass.h>
#include <TRealData.h>

ClassImp(WGlValuator)

/**************************************************************************/

void WGlValuator::_init()
{
  // *** Set all links to 0 ***
  mDx = mDy = 0;

  mMin   = -1000; mMax   = 1000;
  mStepA =  1;    mStepB = 1;
  mFormat = "%6.2f";

  mCbackAlpha = 0;
  mDataMemberInfo = 0; mDataMember = 0;
}

/**************************************************************************/

GledNS::DataMemberInfo* WGlValuator::GetDataMemberInfo()
{
  if(mDataMemberInfo == 0)
    mDataMemberInfo = GledNS::DeduceDataMemberInfo(mCbackAlpha, mCbackMemberName.Data());
  return mDataMemberInfo;
}

TDataMember* WGlValuator::GetDataMember()
{
  if(mDataMember == 0 && mCbackAlpha != 0) {
    TRealData* rd = mCbackAlpha->IsA()->GetRealData((string("m") + mCbackMemberName.Data()).c_str());
    if(rd != 0)
      mDataMember = rd->GetDataMember();
  }
  return mDataMember;
}

Bool_t WGlValuator::DataOK()
{
  GetDataMemberInfo(); GetDataMember();
  return (mDataMemberInfo != 0 && mDataMember != 0);
}

/**************************************************************************/
