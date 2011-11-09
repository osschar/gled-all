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
  mStream << "******************** Logging started at " << GTime(GTime::I_Now).ToDateTimeLocal(false) << " ********************" << endl;

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

  mStream << "******************** Logging stopped at " << GTime(GTime::I_Now).ToDateTimeLocal(false) << " ********************" << endl;
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

#define LEVEL_CHECK(_lvl_) \
  if (mLevel < L_Debug) { \
    if (_lvl_ > mLevel) return; \
  } else { \
    if (_lvl_ > L_Debug + mDebugLevel) return; \
  }

#define LEVEL_NAME(_name_, _lvl_) \
  TString _name_; \
  if (_lvl_ <= L_Debug) { \
    _name_ = lvl_names[_lvl_]; \
  } else { \
    _name_ = lvl_names[L_Debug]; \
    _name_ += char('0' + _lvl_ - L_Debug); \
  }

#define MSG_STRING(_str_, _fmt_) \
  TString _str_; { va_list __ap; va_start(__ap, _fmt_); \
                   _str_.Form(_fmt_, __ap); va_end(__ap); }

void ZLog::Put(Int_t level, const GTime& time, const TString& prefix, const TString& message)
{
  Put(level, time.ToDateTimeLocal(false), prefix, message);
}

void ZLog::Put(Int_t level, const TString& time_string, const TString& prefix, const TString& message)
{
  LEVEL_CHECK(level);
  LEVEL_NAME(lvl_name, level);

  GMutexHolder _lck(mLoggerCond);
  mStream << time_string << " " << lvl_name << " " << prefix << message << endl;
}

void ZLog::Form(Int_t level, const GTime& time, const TString& prefix, const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  Form(level, time.ToDateTimeLocal(false), prefix, fmt, ap);
  va_end(ap);
}

void ZLog::Form(Int_t level, const TString& time_string, const TString& prefix, const char *fmt, ...)
{
  LEVEL_CHECK(level);
  LEVEL_NAME(lvl_name, level);
  MSG_STRING(message, fmt);

  GMutexHolder _lck(mLoggerCond);
  mStream << time_string << " " << lvl_name << " " << prefix << message << endl;
}


//==============================================================================
// ZLog::Helper
//==============================================================================

ZLog::Helper::Helper(ZLog* log, const GTime& when, Int_t lvl, const TString& pfx) :
  m_log(log), m_prefix(pfx), m_level(lvl)
{
  SetTime(when);
}

void ZLog::Helper::SetTime(const GTime& time)
{
  m_time = time;
  m_time_string = m_time.ToDateTimeLocal(false);
}

void ZLog::Helper::Put(const TString& message)
{
  m_log->Put(m_level, m_time_string, m_prefix, message);
}

void ZLog::Helper::Put(Int_t level, const TString& message)
{
  m_log->Put(level, m_time_string, m_prefix, message);
}

void ZLog::Helper::Form(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  m_log->Form(m_level, m_time_string, m_prefix, fmt, ap);
  va_end(ap);

}

void ZLog::Helper::Form(Int_t level, const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  m_log->Form(level, m_time_string, m_prefix, fmt, ap);
  va_end(ap);
}
