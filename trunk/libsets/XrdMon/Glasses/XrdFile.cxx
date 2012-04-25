// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdFile.h"
#include "XrdUser.h"
#include "XrdFile.c7"

// XrdFile

//______________________________________________________________________________
//
//

ClassImp(XrdFile);

//==============================================================================

void XrdFile::_init()
{
  m_dict_id = 0;

  mOpenTime.SetNever();
  mCloseTime.SetNever();
  mLastMsgTime.SetNever();

  mReadStats.Reset();
  mSingleReadStats.Reset();
  mVecReadStats.Reset();
  mVecReadCntStats.Reset();
  mWriteStats.Reset();
  mRTotalMB = mWTotalMB = 0;
  mSizeMB = 0;
}

XrdFile::XrdFile(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

XrdFile::~XrdFile()
{}

//==============================================================================

void XrdFile::AddReadSample(Double_t x)
{
  mReadStats.AddSample(x);
  mSingleReadStats.AddSample(x);
}

void XrdFile::AddVecReadSample(Double_t x, Int_t n)
{
  mReadStats.AddSample(x);
  mVecReadStats.AddSample(x);
  mVecReadCntStats.AddSample(n);
}

void XrdFile::AddWriteSample(Double_t x)
{
  mWriteStats.AddSample(x);
}
