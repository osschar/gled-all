// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_SXrdIoInfo_H
#define XrdMon_SXrdIoInfo_H

#include "Rtypes.h"
#include <vector>

//==============================================================================

class SXrdRwReq
{
public:
  Long64_t          mOffset;
  Int_t             mLength;
  Int_t             mTime;

  SXrdRwReq()  {}
  ~SXrdRwReq() {}

  ClassDefNV(SXrdRwReq, 1);
}; // endclass SXrdRwReq

typedef std::vector<SXrdRwReq> vSXrdRwReq_t;

//==============================================================================

class SXrdRvSubReq
{
public:
  Long64_t          mOffset;
  Int_t             mLength;

  SXrdRvSubReq()  {}
  ~SXrdRvSubReq() {}

  ClassDefNV(SXrdRvSubReq, 1);
}; // endclass SXrdRvSubReq

typedef std::vector<SXrdRvSubReq> vSXrdRvSubReq_t;

//==============================================================================

class SXrdRvReq
{
public:
  Int_t             mNSubReqs;
  Int_t             mTotalLength;
  Int_t             mTime;

  vSXrdRvSubReq_t   mSubReqs;

  SXrdRvReq()  {}
  ~SXrdRvReq() {}

  ClassDefNV(SXrdRvReq, 1);
}; // endclass SXrdRvReq

typedef std::vector<SXrdRvReq> vSXrdRvReq_t;

//==============================================================================

class SXrdIoInfo
{
public:
  vSXrdRwReq_t      mWrites;
  vSXrdRwReq_t      mReads;
  vSXrdRvReq_t      mReadvs;

  SXrdIoInfo()  {}
  ~SXrdIoInfo() {}

  ClassDefNV(SXrdIoInfo, 1);
}; // endclass SXrdIoInfo

#endif
