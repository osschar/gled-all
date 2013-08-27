// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_SXrdServerInfo_H
#define XrdMon_SXrdServerInfo_H

#include "Rtypes.h"
#include "TString.h"

#ifndef __CINT__
class XrdServer;
#endif

class SXrdServerInfo
{
public:
  TString           mHost;
  TString           mDomain;

  SXrdServerInfo()  {}
  ~SXrdServerInfo() {}

#ifndef __CINT__
  void Assign(const XrdServer* s);
#endif

  ClassDefNV(SXrdServerInfo, 1);
}; // endclass SXrdServerInfo

#endif
