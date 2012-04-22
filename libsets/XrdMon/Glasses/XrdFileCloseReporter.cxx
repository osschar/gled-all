// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdFileCloseReporter.h"
#include "Glasses/ZLog.h"
#include "XrdFileCloseReporter.c7"

#include "XrdFile.h"
#include "XrdUser.h"
#include "XrdServer.h"

#include "Gled/GThread.h"

// XrdFileCloseReporter

//______________________________________________________________________________
//
//

ClassImp(XrdFileCloseReporter);

//==============================================================================

void XrdFileCloseReporter::_init()
{}

XrdFileCloseReporter::XrdFileCloseReporter(const Text_t* n, const Text_t* t) :
  ZGlass(n, t),
  mReporterThread(0)
{
  _init();
}

XrdFileCloseReporter::~XrdFileCloseReporter()
{}

//==============================================================================

void XrdFileCloseReporter::FileClosed(XrdFile* file, XrdUser* user, XrdServer* server)
{
  // put into queue and signal condition

  file  ->IncEyeRefCount();
  user  ->IncEyeRefCount();
  server->IncEyeRefCount();

  GMutexHolder _lck(mReporterCond);
  mReporterQueue.push_back(FileUserServer(file, user, server));
  mReporterCond.Signal();
}

//==============================================================================

void XrdFileCloseReporter::ReportLoopInit()
{}

void XrdFileCloseReporter::ReportFileClosed(FileUserServer& fus)
{
  static const Exc_t _eh("XrdFileCloseReporter::ReportFileClosed ");

  ZLog::Helper log(*mLog, ZLog::L_Message, _eh);
  log.Form("LFN=%s", fus.fFile->GetName());
}

void XrdFileCloseReporter::ReportLoopFinalize()
{}

//==============================================================================

void* XrdFileCloseReporter::tl_ReportLoop(XrdFileCloseReporter* r)
{
  r->ReportLoop();
  r->mReporterThread = 0;
  return 0;
}

void XrdFileCloseReporter::ReportLoop()
{
  static const Exc_t _eh("XrdFileCloseReporter::ReportLoop ");

  ReportLoopInit();

  while (true)
  {
    FileUserServer fus;
    {
      GMutexHolder _lck(mReporterCond);
      if (mReporterQueue.empty())
      {
        mReporterCond.Wait();
      }
      fus = mReporterQueue.front();
      mReporterQueue.pop_front();
    }

    ReportFileClosed(fus);

    fus.fFile  ->DecEyeRefCount();
    fus.fUser  ->DecEyeRefCount();
    fus.fServer->DecEyeRefCount();
  }
}

//==============================================================================

void XrdFileCloseReporter::StartReporter()
{
  static const Exc_t _eh("XrdFileCloseReporter::StartReporter ");

  {
    GLensReadHolder _lck(this);
    if (mReporterThread)
      throw _eh + "already running.";

    mReporterThread = new GThread("XrdFileCloseReporter-Reporter",
                                  (GThread_foo) tl_ReportLoop, this,
                                  false);
  }
  mReporterThread->SetNice(20);
  mReporterThread->Spawn();
}

void XrdFileCloseReporter::StopReporter()
{
  static const Exc_t _eh("XrdFileCloseReporter::StopReporter ");

  GThread *thr = 0;
  {
    GLensReadHolder _lck(this);
    if ( ! GThread::IsValidPtr(mReporterThread))
      throw _eh + "not running.";
    thr = mReporterThread;
    GThread::InvalidatePtr(mReporterThread);
  }
  thr->Cancel();
  thr->Join();
  ReportLoopFinalize();
  {
    GLensReadHolder _lck(this);
    mReporterQueue.clear();
    mReporterThread = 0;
  }
}
