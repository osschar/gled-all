// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SXrdUserInfo.h"
#include "Glasses/XrdUser.h"

// SXrdUserInfo

//______________________________________________________________________________
//
//

ClassImp(SXrdUserInfo);

//==============================================================================

void SXrdUserInfo::Assign(const XrdUser* s)
{
  mName = s->RefName();

  mRealName = s->RefRealName();
  mDN = s->RefDN();
  mVO = s->RefVO();
  mRole = s->RefRole();
  mGroup = s->RefGroup();
  mServerUsername = s->RefServerUsername();
  mFromHost = s->RefFromHost();
  mFromDomain = s->RefFromDomain();
  mAppInfo = s->RefAppInfo();
  mLoginTime = s->RefLoginTime().GetSec();
  bNumericHost = s->GetNumericHost();
}
