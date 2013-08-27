// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

// To use aclic, from root prompt, load as:
//   .L SXrdClasses.h+
//
// To build library by hand:
//   rootcint -f SXrdClasses_Dict.cxx -c -p SXrdClasses.h SXrdClasses_LinkDef.h
//   g++ -o libSXrdClasses.so -shared -fPIC `root-config --cflags` SXrdClasses_Dict.cxx
// In root:
//   gSystem->Load("libSXrdClasses")
//   // or put it in rootrc
//
// For analysis of IOV data, stored in SXrdIoInfo structs, see function
// dump_xrdfar_tree_ioinfo().

#include <Rtypes.h>
#include <TMath.h>
#include <TString.h>

//==============================================================================

class SRange
{
public:
  Double_t	mMin;   // X{GS}
  Double_t	mMax;   // X{GS}
  Double_t      mSumX;  // X{GS}
  Double_t      mSumX2; // X{GS}
  ULong64_t     mN;     // X{GS}

public:
  SRange() { Reset(); }

  void Reset();
  void AddSample(Double_t x);

  Double_t GetAverage() const;
  Double_t GetSigma() const;

  ClassDefNV(SRange, 1);
}; // endclass SRange

//------------------------------------------------------------------------------

inline void SRange::Reset()
{
  mMin = mMax = mSumX = mSumX2 = 0;
  mN = 0;
}

inline void SRange::AddSample(Double_t x)
{
  if (mN == 0) {
    mMin = mMax = x;
  } else {
    if (x < mMin) mMin = x;
    if (x > mMax) mMax = x;
  }
  mSumX  += x;
  mSumX2 += x*x;
  ++mN;
}

inline Double_t SRange::GetAverage() const
{
  if (mN > 0)
    return mSumX / mN;
  else
    return 0;
}

inline Double_t SRange::GetSigma() const
{
  if (mN > 0)
    return TMath::Sqrt((mSumX2 - mSumX*mSumX/mN)/mN);
  else
    return 0;
}

//==============================================================================

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

  ClassDefNV(SXrdFileInfo, 1);
}; // endclass SXrdFileInfo

//==============================================================================

class SXrdServerInfo
{
public:
  TString           mHost;
  TString           mDomain;

  SXrdServerInfo()  {}
  ~SXrdServerInfo() {}

  ClassDefNV(SXrdServerInfo, 1);
}; // endclass SXrdServerInfo

//==============================================================================

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
  TString           mAppInfo;
  Long64_t          mLoginTime;
  Bool_t            bNumericHost;

  SXrdUserInfo()  {}
  ~SXrdUserInfo() {}

  ClassDefNV(SXrdUserInfo, 1);
}; // endclass SXrdUserInfo

//==============================================================================

class SXrdReq
{
public:
  enum Req_e { R_Write, R_Read, R_VecRead };

private:
  Long64_t mOffset;   // Overloaded for R_VecRead
  Int_t    mLength;
  Int_t    mTime;

  void set_req_count(UShort_t rc)  { assign_offset(0xffff, 32, rc); }
  void set_reqs_lost(UShort_t rm)  { assign_offset(0x7fff, 48, rm); }

  void assign_offset(Long64_t mask, Int_t shift, Long64_t value)
  {
    value <<= shift;
    mask  <<= shift;
    mOffset &= ~mask;
    mOffset |=  value;
  }

public:
  SXrdReq() : mOffset(0), mLength(0), mTime(0) {}

  SXrdReq(Long64_t off, Int_t len, Int_t time) :
    mOffset(off), mLength(len), mTime(time) {}

  SXrdReq(Int_t index, UShort_t n_seg, Int_t len, Int_t time) :
    mOffset((1ll << 63) | (0xffffffffll & index) | (Long64_t(n_seg) << 32)),
    mLength(len), mTime(time) {}

  void IncLength(Int_t len)         { mLength += len; }
  void IncSubReqCount(UShort_t cnt) { set_req_count(SubReqCount() + cnt); }
  void IncSubReqsLost(UShort_t cnt) { set_reqs_lost(SubReqsLost() + cnt); }

  // Access functions

  Req_e Type() const
  {
    if (mOffset < 0) return R_VecRead;
    if (mLength < 0) return R_Write; else return R_Read;
  }
  Int_t  Length() const { return mLength < 0 ? -mLength : mLength; }
  Int_t  Time()   const { return mTime; }

  // For Read and Write
  Long64_t Offset() const { return mOffset < 0 ? -1 : mOffset; }

  // For VecRead
  Int_t    SubReqIndex() const   { return mOffset & 0xffffffff; }
  UShort_t SubReqCount() const   { return (mOffset >> 32) & 0xffff; }
  UShort_t SubReqsLost() const   { return (mOffset >> 48) & 0x7fff; }
  UShort_t SubReqsStored() const { return SubReqIndex() >= 0 ? SubReqCount() - SubReqsLost() : 0; }

  const char* TypeName() const
  {
    if (mOffset < 0) return "VecRead";
    if (mLength < 0) return "Write"; else return "Read";
  }

  ClassDefNV(SXrdReq, 1);
}; // endclass SXrdReq

typedef std::vector<SXrdReq> vSXrdReq_t;
typedef vSXrdReq_t::iterator vSXrdReq_i;

//==============================================================================

class SXrdIoInfo
{
public:
  vSXrdReq_t            mReqs;
  Int_t                 mNErrors;

  // Details for unpacked vector reads.
  std::vector<Long64_t> mOffsetVec;
  std::vector<Int_t>    mLengthVec;

  // ----------------------------------------------------------------

  SXrdIoInfo() : mNErrors(0) {}
  ~SXrdIoInfo() {}

  void Dump(Int_t level=1);

  ClassDefNV(SXrdIoInfo, 1);
}; // endclass SXrdIoInfo

//------------------------------------------------------------------------------

inline void SXrdIoInfo::Dump(Int_t level)
{
  printf("SXrdIoInfo::Dump() N_reqs = %zu, N_vec_read_subreqs = %zu, N_errors = %d\n",
	 mReqs.size(), mOffsetVec.size(), mNErrors);

  if (level <= 0) return;

  int cnt = 0;
  for (vSXrdReq_i i = mReqs.begin(); i != mReqs.end(); ++i, ++cnt)
  {
    printf("%3d  %-7s ", cnt, i->TypeName());
    switch (i->Type())
    {
      case SXrdReq::R_Write:
      {
	printf("t=%-5d l=%-7d o=%lld\n", i->Time(), -i->Length(), i->Offset());
	break;
      }
      case SXrdReq::R_Read:
      {
	printf("t=%-5d l=%-7d o=%lld\n", i->Time(),  i->Length(), i->Offset());
	break;
      }
      case SXrdReq::R_VecRead:
      {
	Int_t sr_idx = i->SubReqIndex();

	printf("t=%-5d l=%-7d n_req_cnt=%-5hu n_reqs_lost=%-5hu idx=%d\n",
	       i->Time(), i->Length(), i->SubReqCount(), i->SubReqsLost(),
	       sr_idx);

	if (level > 1 && sr_idx >= 0)
	{
	  Int_t max = sr_idx + i->SubReqsStored();
	  Int_t j   = 0;
	  for (Int_t si = i->SubReqIndex(); si < max; ++si, ++j)
	  {
	    printf("             i=%-5d l=%-7d o=%lld\n", j, mLengthVec[si], mOffsetVec[si]);
	  }
	}
	break;
      }
    }
  }
}

//==============================================================================

#include "TTree.h"

inline void dump_xrdfar_tree_ioinfo(TTree *t, Long64_t ev)
{
  if (ev < 0 || ev >= t->GetEntriesFast())
  {
    fprintf(stderr, "Error, event %lld out of range (max=%lld).\n",
	    ev, t->GetEntriesFast());
    return;
  }

  SXrdIoInfo *xp = 0;
  t->SetBranchAddress("I.", &xp);

  t->GetEntry(ev);

  xp->Dump(2);
}
