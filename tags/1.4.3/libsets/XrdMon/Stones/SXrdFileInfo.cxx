// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SXrdFileInfo.h"
#include "Glasses/XrdFile.h"

// SXrdFileInfo

//______________________________________________________________________________
//
//

ClassImp(SXrdFileInfo);

//==============================================================================

void SXrdFileInfo::Assign(const XrdFile* s)
{
  mName = s->RefName();

  mOpenTime = s->RefOpenTime().GetSec();
  mCloseTime = s->RefCloseTime().GetSec();

  mReadStats = s->RefReadStats();
  mSingleReadStats = s->RefSingleReadStats();
  mVecReadStats = s->RefVecReadStats();
  mVecReadCntStats = s->RefVecReadCntStats();
  mWriteStats = s->RefWriteStats();

  mRTotalMB = s->GetRTotalMB();
  mWTotalMB = s->GetWTotalMB();
  mSizeMB = s->GetSizeMB();
}
