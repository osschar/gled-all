// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_SXrdFileInfo_H
#define XrdMon_SXrdFileInfo_H

#include "Rtypes.h"
#include "TString.h"

#include "Stones/SRange.h"

#ifndef __CINT__
class XrdFile;
#endif

class SXrdFileInfo
{
public:
  TString         mName;

  Long64_t        mOpenTime;
  Long64_t        mCloseTime;

  SRange          mReadStats;
  SRange          mSingleReadStats;
  SRange          mVecReadStats;
  SRange          mVecReadCntStats;
  SRange          mWriteStats;

  Double_t        mRTotalMB;
  Double_t        mWTotalMB;
  Double_t        mSizeMB;

  SXrdFileInfo()  {}
  ~SXrdFileInfo() {}

#ifndef __CINT__
  void Assign(const XrdFile* s);
#endif

  ClassDefNV(SXrdFileInfo, 1);
}; // endclass SXrdFileInfo

#endif
