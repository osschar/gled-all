// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_XrdFileCloseReporter_H
#define XrdMon_XrdFileCloseReporter_H

#include "Glasses/ZGlass.h"
#include "Gled/GCondition.h"

class XrdFile;
class ZLog;

class GThread;


class XrdFileCloseReporter : public ZGlass
{
  MAC_RNR_FRIENDS(XrdFileCloseReporter);

private:
  void _init();

protected:
  ZLink<ZLog>       mLog;     // X{GS} L{}

  GThread          *mReporterThread; //!
  GCondition        mReporterCond;   //!
  list<XrdFile*>    mReporterQueue;  //!

  static void* tl_ReportLoop(XrdFileCloseReporter* r);
  void ReportLoop();

  virtual void ReportLoopInit();
  virtual void ReportFileClosed(XrdFile* file);
  virtual void ReportLoopFinalize();

public:
  XrdFileCloseReporter(const Text_t* n="XrdFileCloseReporter", const Text_t* t=0);
  virtual ~XrdFileCloseReporter();

  void FileClosed(XrdFile* file);

  void StartReporter(); // X{Ed} 7 MButt()
  void StopReporter();  // X{Ed} 7 MButt()

#include "XrdFileCloseReporter.h7"
  ClassDef(XrdFileCloseReporter, 1);
}; // endclass XrdFileCloseReporter

#endif
