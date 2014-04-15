// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_SXrdUserInfo_H
#define XrdMon_SXrdUserInfo_H

#include "Rtypes.h"
#include "TString.h"

#ifndef __CINT__
class XrdUser;
#endif

class SXrdUserInfo
{
public:
  TString           mName;

  TString           mRealName;
  TString           mDN;
  TString           mVO;
  TString           mRole;
  TString           mGroup;
  TString           mServerUsername;
  TString           mFromHost;
  TString           mFromDomain;
  TString           mProtocol;
  TString           mAppInfo;
  Long64_t          mLoginTime;
  Bool_t            bNumericHost;

  SXrdUserInfo()  {}
  ~SXrdUserInfo() {}

#ifndef __CINT__
  void Assign(const XrdUser* s);
#endif

  ClassDefNV(SXrdUserInfo, 2);
}; // endclass SXrdUserInfo

#endif
