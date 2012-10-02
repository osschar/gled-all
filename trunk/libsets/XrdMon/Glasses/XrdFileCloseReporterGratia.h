// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_XrdFileCloseReporterGratia_H
#define XrdMon_XrdFileCloseReporterGratia_H

#include <Glasses/XrdFileCloseReporter.h>

struct sockaddr;

class XrdFileCloseReporterGratia : public XrdFileCloseReporter
{
  MAC_RNR_FRIENDS(XrdFileCloseReporterGratia);

private:
  void _init();

protected:
  TString           mUdpHost; // X{GS} 7 Textor()
  UShort_t          mUdpPort; // X{GS} 7 Value()

  Bool_t            bDomenicoIds;    //! X{GS} 7 Bool()

  Int_t             mReporterSocket; //!

  Long64_t          mLastUidBase;    //!
  Long64_t          mLastUidInner;   //!

  struct sockaddr  *mSAddr;          //!

  virtual void ReportLoopInit();
  virtual void ReportFileClosed(FileUserServer& fus);
  virtual void ReportLoopFinalize();

public:
  XrdFileCloseReporterGratia(const Text_t* n="XrdFileCloseReporterGratia", const Text_t* t=0);
  virtual ~XrdFileCloseReporterGratia();

#include "XrdFileCloseReporterGratia.h7"
  ClassDef(XrdFileCloseReporterGratia, 1);
}; // endclass XrdFileCloseReporterGratia

#endif
