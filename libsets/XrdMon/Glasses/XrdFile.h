// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_XrdFile_H
#define XrdMon_XrdFile_H

#include <Glasses/ZGlass.h>
#include <Stones/SRange.h>
#include <Stones/SXrdIoInfo.h>
#include <Gled/GTime.h>

class XrdUser;

class XrdFile : public ZGlass
{
  MAC_RNR_FRIENDS(XrdFile);
  friend class XrdServer;

private:
  UInt_t          m_dict_id;        //!

  void _init();

protected:
  ZLink<XrdUser>  mUser;            // X{GS} L{}

  GTime           mOpenTime;        // X{GRSQ} 7 TimeOut()
  GTime           mCloseTime;       // X{GRSQ} 7 TimeOut()
  GTime           mLastMsgTime;     // X{GRSQ} 7 TimeOut()

  SRange          mReadStats;       // X{GR}   7 StoneOutput(-width=>32, Fmt=>"%.3fMB - N=%llu, A=%.3f, S=%.3f", Args=>[SumX, N, Average, Sigma])
  SRange          mSingleReadStats; // X{GR}   7 StoneOutput(-width=>32, Fmt=>"%.3fMB - N=%llu, A=%.3f, S=%.3f", Args=>[SumX, N, Average, Sigma])
  SRange          mVecReadStats;    // X{GR}   7 StoneOutput(-width=>32, Fmt=>"%.3fMB - N=%llu, A=%.3f, S=%.3f", Args=>[SumX, N, Average, Sigma])
  SRange          mVecReadCntStats; // X{GR}   7 StoneOutput(-width=>32, Fmt=>"A=%.3f, S=%.3f",                  Args=>[Average, Sigma])
  SRange          mWriteStats;      // X{GR}   7 StoneOutput(-width=>32, Fmt=>"%.3fMB - N=%llu, A=%.3f, S=%.3f", Args=>[SumX, N, Average, Sigma])

  Double_t        mRTotalMB;        // X{GSD}  7 ValOut(-join=>1)
  Double_t        mWTotalMB;        // X{GSD}  7 ValOut()
  Double_t        mSizeMB;          // X{GS}   7 ValOut()

  SXrdIoInfo      mIoInfo;        //! X{rp}
  // State variables
  Short_t         mExpectedReadVSegs;
  UChar_t         mLastVSeq;
  Bool_t          bInReadV;
  // XXXX These two should go to IoInfo, probably
  TString         mErrors;
  Int_t           mNErrors;

  void begin_readv(Int_t n_segments, Int_t total_length, Int_t time, UChar_t vseq);
  void extend_readv(Int_t n_segments, Int_t total_length);
  void end_readv();

public:
  XrdFile(const Text_t* n="XrdFile", const Text_t* t=0);
  virtual ~XrdFile();

  Bool_t IsOpen() const { return mCloseTime.IsNever() && ! mOpenTime.IsNever(); }

  // These should be protected ... hmmh, doesn't matter.
  void AddReadSample(Double_t x);
  void AddVecReadSample(Double_t x, Int_t n);
  void AddWriteSample(Double_t x);

  // Interface for registration of IO info (when enabled).

  void RegisterReadOrWrite(Long64_t offset, Int_t length, const GTime& time); // ????
  void RegisterRead (Long64_t offset, Int_t length, const GTime& time);
  void RegisterWrite(Long64_t offset, Int_t length, const GTime& time);

  void RegisterReadV(Int_t n_segments, Int_t total_length, const GTime& time, UChar_t vseq);
  void RegisterReadU(Int_t n_segments, Int_t total_length, const GTime& time, UChar_t vseq);
  void RegisterReadVSeg(Long64_t offset, Int_t length);

  void RegisterFileClose(); // ? heh, what happens on automatic closing?

#include "XrdFile.h7"
  ClassDef(XrdFile, 1);
}; // endclass XrdFile

#endif
