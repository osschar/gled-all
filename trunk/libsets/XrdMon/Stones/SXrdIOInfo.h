// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_SXrdIOInfo_H
#define XrdMon_SXrdIOInfo_H

#include "Rtypes.h"
#include <vector>

//==============================================================================

class SXrdRWReq
{
public:
  Long64_t          mOffset;
  Int_t             mLength;
  Int_t             mTime;

  SXrdRWReq()  {}
  ~SXrdRWReq() {}

  ClassDefNV(SXrdRWReq, 1);
}; // endclass SXrdRWReq

typedef std::vector<SXrdRWReq> vSXrdRWReq_t;

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

class SXrdIOInfo
{
public:
  vSXrdRWReq_t      mWrites;
  vSXrdRWReq_t      mReads;
  vSXrdRvReq_t      mReadvs;

  SXrdIOInfo()  {}
  ~SXrdIOInfo() {}

  ClassDefNV(SXrdIOInfo, 1);
}; // endclass SXrdIOInfo

#endif
