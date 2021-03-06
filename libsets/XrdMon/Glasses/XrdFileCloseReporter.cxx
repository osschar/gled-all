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

#include "TUUID.h"

// XrdFileCloseReporter

//______________________________________________________________________________
//
// Core functionality for classes that perform some kind of reporting when an
// XrdFile is closed.
//
// Disables thread canellation whenever calling virtual functions in
// sub-classes: ReportLoopInit(), ReportFileClosed() and
// ReportCondWaitTimeout(). ReportLoopFinalize() is called from cancellation
// handler anyway.

ClassImp(XrdFileCloseReporter);

//==============================================================================

void XrdFileCloseReporter::_init()
{
  mCondWaitSec = 0;
  bFixedUuid   = false;
  mNProcessedTotal = mNProcessed = mNQueued = 0;
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

void XrdFileCloseReporter::FixUuidString(const TString& uuid)
{
  // Set a fixed UUID string to be prefixed to unique_id field of a
  // file-access report. The complete format is still "%s-%llx".
  // This function can only be called when reporter thread is not running.

  static const Exc_t _eh("XrdFileCloseReporter::FixUuidString ");

  if (bRunning)
    throw _eh + "Can not be called while reporter thread is running.";

  bFixedUuid = true;
  mUuid      = uuid;

  Stamp(FID());
}

void XrdFileCloseReporter::AutomaticUuidString()
{
  // Automatically generate UUID prefix for unique_id field at service start.
  // This function can only be called when reporter thread is not running.

  static const Exc_t _eh("XrdFileCloseReporter::AutomaticUuidString ");

  if (bRunning)
    throw _eh + "Can not be called while reporter thread is running.";

  bFixedUuid = false;
  mUuid      = "";

  Stamp(FID());
}

//==============================================================================

void XrdFileCloseReporter::FileClosed(XrdFile* file, XrdUser* user, XrdServer* server)
{
  // Put the triplet into queue and signal condition.

  if (! bRunning)
    return;

  GThread::CancelDisabler _cd;

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
  //
  // Called without lock, cancellation disabled.
}

void XrdFileCloseReporter::ReportFileClosed(FileUserServer& fus)
{
  // Sub-classes should override this to process files that have been just
  // closed. No need to call the parent-class implementation from there, here
  // we just write out to the log.
  //
  // Called without lock, cancellation disabled.

  static const Exc_t _eh("XrdFileCloseReporter::ReportFileClosed ");

  ZLog::Helper log(*mLog, ZLog::L_Message, _eh);
  log.Form("LFN=%s", fus.fFile->GetName());
}

void XrdFileCloseReporter::ReportCondWaitTimeout()
{
  // Sub-classes should override this to perform periodic checks.
  // If mCondWaitSec = 0 then this function never gets called.
  //
  // Called without lock, cancellation disabled.
}

void XrdFileCloseReporter::ReportLoopFinalize()
{
  // Sub-classes should override this to perform cleanup.
  // This virtual is called from the cleanup-handler of the worker thread.
  // No need to call the parent-class implementation there.
  //
  // Called without lock from thread cleanup function.
}

//==============================================================================

void* XrdFileCloseReporter::tl_ReportLoop(XrdFileCloseReporter* r)
{
  GThread *thr = GThread::Self();

  r->mSaturn->register_detached_thread(r, thr);

  thr->CleanupPush((GThread_cu_foo) cu_ReportLoop, r);

  {
    GThread::CancelDisabler _cd;

    {
      GLensReadHolder _lck(r);
      if ( ! r->bFixedUuid)
      {
        r->mUuid = GledNS::get_uuid_string();
      }
      r->mNProcessed = 0;
      r->bRunning = true;
      r->Stamp(r->FID());
    }

    r->ReportLoopInit();
  }

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
    GMutexHolder  _lck(mReporterCond);

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
	  GThread::CancelDisabler _cd;

	  _lck.Unlock();

	  ReportCondWaitTimeout();
	  continue;
	}
      }
    }

    GThread::CancelDisabler _cd;

    fus = mReporterQueue.front();
    mReporterQueue.pop_front();
    --mNQueued;

    _lck.Unlock();

    ReportFileClosed(fus);
    ++mNProcessedTotal;
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
  delete thr;
}
