// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZLog.h"
#include "ZLog.c7"

#include "Gled/GThread.h"

#include <time.h>

// ZLog

//______________________________________________________________________________
//
// Fast, preliminary implementation to plug a hole in XrdMonSucker.
// Needs more work ...
//
// Short term:
// - Various helper / access functions to make it easier to use.

// Medium term:
// - Log rotation,
// - Do output in the dedicated thread. (not 100% sure this is smart)
//
// Long term:
// - GUI (yes, right).

ClassImp(ZLog);

//==============================================================================

void ZLog::_init()
{
  mFileName = mName + ".log";
  mLevel = L_Message;
  mDebugLevel = 0;
  mLoggerThread = 0;
}

ZLog::ZLog(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

ZLog::~ZLog()
{}

//==============================================================================

// void ZLog::operator()(Int_t level, const char* sth)
// {
//   clock_t clk = clock();
//   printf("Clocka %lu %lu\n", clk, clk / CLOCKS_PER_SEC);
//   printf("Jebojebo, got lvl=%d, txt='%s'\n", level, sth);
// }

//==============================================================================

void ZLog::StartLogging()
{
  static const Exc_t _eh("ZLog::StartLogging ");

  GMutexHolder _lck(mLoggerCond);

  if (mLoggerThread != 0)
    throw _eh + "Logging already active.";

  mStream.open(mFileName, ios_base::out | ios_base::app);
  mStream << "********** Logging starts at " << GTime(GTime::I_Now).ToDateTimeLocal(false) << " **********" << endl;

  mLoggerThread = new GThread("ZLog-LogLoop", (GThread_foo) tl_LogLoop, this);
  mLoggerThread->SetNice(20);
  mLoggerThread->Spawn();
}

void ZLog::StopLogging()
{
  static const Exc_t _eh("ZLog::StopLogging ");

  GMutexHolder _lck(mLoggerCond);

  if ( ! GThread::IsValidPtr(mLoggerThread))
    throw _eh + "Logging not active.";

  GThread *thr = mLoggerThread;
  GThread::InvalidatePtr(mLoggerThread);
  thr->Cancel();
  thr->Join();

  mStream << "********** Logging ends at " << GTime(GTime::I_Now).ToDateTimeLocal(false) << " **********" << endl;
  mStream.close();
  mLoggerThread = 0;
}

//------------------------------------------------------------------------------

void ZLog::tl_LogLoop(ZLog* log)
{
  log->LogLoop();
}

void ZLog::LogLoop()
{
  while (true)
  {
    // Take care of log rotation.
    // ??? Should also have message queue for log entries?
    GTime::SleepMiliSec(100*1000);
  }
}

//==============================================================================

namespace
{
  static const char *lvl_names[] = { "FTL", "ERR", "WRN", "MSG", "INF", "DBG" };
}

void ZLog::Put(Int_t level, const GTime& time, const TString& prefix, const TString& message)
{
  if (mLevel < L_Debug)
  {
    if (level > mLevel) return;
  }
  else
  {
    if (level > L_Debug + mDebugLevel)
      return;
  }

  TString lvl_name;
  if (level <= L_Debug)
  {
    lvl_name = lvl_names[level];
  }
  else
  {
    lvl_name = lvl_names[L_Debug];
    lvl_name += char('0' + level - L_Debug);
  }

  GMutexHolder _lck(mLoggerCond);
  mStream << time.ToDateTimeLocal(false) << " " << lvl_name << " " << prefix << message << endl;
}
