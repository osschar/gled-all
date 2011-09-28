// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_XrdFile_H
#define CmsGridViz_XrdFile_H

#include <Glasses/ZGlass.h>
#include <Stones/SRange.h>
#include <Gled/GTime.h>

class XrdUser;

class XrdFile : public ZGlass
{
  MAC_RNR_FRIENDS(XrdFile);

private:
  void _init();

protected:
  ZLink<XrdUser>  mUser;        // X{GS} L{}

  GTime           mOpenTime;    // X{GRSQ} 7 TimeOut()
  GTime           mCloseTime;   // X{GRSQ} 7 TimeOut()
  GTime           mLastMsgTime; // X{GRSQ} 7 TimeOut()

  SRange          mReadStats;   // X{GR}   7 StoneOutput(-width=>32, Fmt=>"%fMB - N=%llu, A=%f, S=%f", Args=>[SumX, N, Average, Sigma])
  SRange          mWriteStats;  // X{GR}   7 StoneOutput(-width=>32, Fmt=>"%fMB - N=%llu, A=%f, S=%f", Args=>[SumX, N, Average, Sigma])

  Double_t        mRTotalMB;    // X{GSD}  7 ValOut()
  Double_t        mWTotalMB;    // X{GSD}  7 ValOut()

public:
  XrdFile(const Text_t* n="XrdFile", const Text_t* t=0);
  virtual ~XrdFile();

  Bool_t IsOpen() const { return mCloseTime.IsNever() && ! mOpenTime.IsNever(); }

  void AddReadSample(Double_t x);
  void AddWriteSample(Double_t x);

#include "XrdFile.h7"
  ClassDef(XrdFile, 1);
}; // endclass XrdFile

#endif
