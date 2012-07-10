// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_XrdFileCloseReporterAmq_H
#define XrdMon_XrdFileCloseReporterAmq_H

#include <Glasses/XrdFileCloseReporter.h>

struct sockaddr;

class XrdFileCloseReporterAmq : public XrdFileCloseReporter
{
  MAC_RNR_FRIENDS(XrdFileCloseReporterAmq);

private:
  void _init();

protected:
  TString           mUdpHost; // X{GS} 7 Textor()
  UShort_t          mUdpPort; // X{GS} 7 Value()

  Int_t             mReporterSocket; //!

  Long64_t          mLastUidBase;    //!
  Long64_t          mLastUidInner;   //!

  struct sockaddr  *mSAddr;          //!

  virtual void ReportLoopInit();
  virtual void ReportFileClosed(FileUserServer& fus);
  virtual void ReportLoopFinalize();

public:
  XrdFileCloseReporterAmq(const Text_t* n="XrdFileCloseReporterAmq", const Text_t* t=0);
  virtual ~XrdFileCloseReporterAmq();

#include "XrdFileCloseReporterAmq.h7"
  ClassDef(XrdFileCloseReporterAmq, 1);
}; // endclass XrdFileCloseReporterAmq

#endif
