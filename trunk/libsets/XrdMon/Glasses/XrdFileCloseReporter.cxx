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
// Core functionality for classes that perform some kind of reporting when an
// XrdFile is closed.
//
// Should activate cancel-disabler whenever calling out to sub-classes, in
// particular, to ReportFileClosed() and
// ReportCondWaitTimeout(). ReportLoopFinalize() is called from cancellation
// handler anyway. Let's hope we don't get cancelled during init ... duh,
// disable it there as well, for symmetry if nothing else.

ClassImp(XrdFileCloseReporter);

//==============================================================================

void XrdFileCloseReporter::_init()
{
  mCondWaitSec = 0;
  mNProcessed = mNQueued = 0;
  bRunning = false;
}

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
  // Put the triplet into queue and signal condition.

  if (! bRunning)
    return;

  file  ->IncEyeRefCount();
  user  ->IncEyeRefCount();
  server->IncEyeRefCount();

  GMutexHolder _lck(mReporterCond);
  mReporterQueue.push_back(FileUserServer(file, user, server));
  ++mNQueued;
  mReporterCond.Signal();
}

//==============================================================================

void XrdFileCloseReporter::ReportLoopInit()
{
  // Sub-classes should override this to perform one-time initialization.
  // This virtual is called from the startup-handler of the worker thread.
  // No need to call the parent-class implementation there.
}

void XrdFileCloseReporter::ReportFileClosed(FileUserServer& fus)
{
  // Sub-classes should override this to process files that have been just
  // closed. No need to call the parent-class implementation from there, here
  // we just write out to the log.

  static const Exc_t _eh("XrdFileCloseReporter::ReportFileClosed ");

  ZLog::Helper log(*mLog, ZLog::L_Message, _eh);
  log.Form("LFN=%s", fus.fFile->GetName());
}

void XrdFileCloseReporter::ReportCondWaitTimeout()
{
  // Sub-classes should override this to perform periodic checks.
  // If mCondWaitSec = 0 then this function never gets called.
}

void XrdFileCloseReporter::ReportLoopFinalize()
{
  // Sub-classes should override this to perform cleanup.
  // This virtual is called from the cleanup-handler of the worker thread.
  // No need to call the parent-class implementation there.
}

//==============================================================================

void* XrdFileCloseReporter::tl_ReportLoop(XrdFileCloseReporter* r)
{
  GThread *thr = GThread::Self();

  r->mSaturn->register_detached_thread(r, thr);

  thr->CleanupPush((GThread_cu_foo) cu_ReportLoop, r);

  {
    GLensReadHolder _lck(r);
    r->bRunning = true;
    r->Stamp(r->FID());
  }

  r->ReportLoopInit();

  r->ReportLoop();

  return 0;
}

void XrdFileCloseReporter::cu_ReportLoop(XrdFileCloseReporter* r)
{
  r->ReportLoopFinalize();

  r->mSaturn->unregister_detached_thread(r, GThread::Self());

  {
    GLensReadHolder _lck(r);
    r->bRunning = false;
  }

  r->DrainQueue();

  {
    GLensReadHolder _lck(r);
    r->mReporterThread = 0;
    r->Stamp(r->FID());
  }
}

void XrdFileCloseReporter::ReportLoop()
{
  static const Exc_t _eh("XrdFileCloseReporter::ReportLoop ");

  while (true)
  {
    FileUserServer fus;
    {
      GMutexHolder _lck(mReporterCond);
      if (mReporterQueue.empty())
      {
	if (mCondWaitSec <= 0)
	{
	  mReporterCond.Wait();
	}
	else
	{
	  if (mReporterCond.TimedWaitUntil(GTime::ApproximateTime() + GTime(mCondWaitSec)) == 1)
	  {
	    ReportCondWaitTimeout();
	    continue;
	  }
	}
      }
      fus = mReporterQueue.front();
      mReporterQueue.pop_front();
      --mNQueued;
    }

    ReportFileClosed(fus);
    ++mNProcessed;

    fus.fFile  ->DecEyeRefCount();
    fus.fUser  ->DecEyeRefCount();
    fus.fServer->DecEyeRefCount();
  }
}

void XrdFileCloseReporter::DrainQueue()
{
  GMutexHolder _lck(mReporterCond);
  while (! mReporterQueue.empty())
  {
    FileUserServer fus = mReporterQueue.front();
    fus.fFile  ->DecEyeRefCount();
    fus.fUser  ->DecEyeRefCount();
    fus.fServer->DecEyeRefCount();
    mReporterQueue.pop_front();
  }
  mNQueued = 0;
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
                                  false, true);
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
  thr->ClearDetachOnExit();
  thr->Cancel();
  thr->Join();
}
