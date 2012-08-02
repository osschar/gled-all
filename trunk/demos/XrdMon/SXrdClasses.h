// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

// To use aclic, from root prompt, load as:
//  .L SXrdClasses.h+

// To build library by hand:
// rootcint -f SXrdClasses_Dict.cxx -c -p SXrdClasses.h SXrdClasses_LinkDef.h
// g++ -o libSXrdClasses.so -shared -fPIC `root-config --cflags` SXrdClasses_Dict.cxx
// In root:
//   gSystem->Load("libSXrdClasses")
//   // or put it in rootrc

#include <Rtypes.h>
#include <TMath.h>
#include <TString.h>

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
