// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZLog.h"
#include "ZLog.c7"

#include "Gled/GThread.h"

#include "TSystem.h"
#include "Varargs.h"

#include <time.h>

// ZLog

//______________________________________________________________________________
//
// Fast, preliminary implementation to plug a hole in XrdMonSucker.
// Needs more work ...
//
// Short term:
// - Various helper / access functions to make it easier to use.
//
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

  {
    GMutexHolder _lck(mLoggerCond);

    if (mLoggerThread != 0)
      throw _eh + "Logging already active.";

    mStream.open(mFileName, ios_base::out | ios_base::app);
    if (mStream.fail())
      throw _eh + "Opening of log '" + mFileName + "' failed.";

    mStream << "******************** Logging started at " << GTime::ApproximateTime().ToDateTimeLocal(false) << " ********************" << endl;

    mLoggerThread = new GThread("ZLog-LogLoop", (GThread_foo) tl_LogLoop, this);
    mLoggerThread->SetNice(20);
    mLoggerThread->Spawn();
  }

  {
    GLensReadHolder _rdlck(this);
    bLogActive = true;
    Stamp(FID());
  }
}

void ZLog::StopLogging()
{
  static const Exc_t _eh("ZLog::StopLogging ");

  {
    GMutexHolder _lck(mLoggerCond);

    if ( ! GThread::IsValidPtr(mLoggerThread))
      throw _eh + "Logging not active.";

    GThread *thr = mLoggerThread;
    GThread::InvalidatePtr(mLoggerThread);
    thr->Cancel();
    thr->Join();

    mStream << "******************** Logging stopped at " << GTime::ApproximateTime().ToDateTimeLocal(false) << " ********************" << endl;
    mStream.close();
    mLoggerThread = 0;
  }

  {
    GLensReadHolder _rdlck(this);
    bLogActive = false;
    Stamp(FID());
  }

}

void ZLog::RotateLog()
{
  static const Exc_t _eh("ZLog::RotateLog ");

  GMutexHolder _lck(mLoggerCond);

  if ( ! GThread::IsValidPtr(mLoggerThread))
    throw _eh + "Logging not active.";

  if (gSystem->AccessPathName(mFileName, kFileExists) == false)
  {
    TString newfile = mFileName + "." + GTime::ApproximateTime().ToDateLocal();
    if (gSystem->AccessPathName(newfile, kFileExists) == false)
    {
      Int_t cnt = 1;
      newfile += ".";
      TString tstr;
      do
      {
        tstr = newfile;
        tstr += cnt++;
      }
      while (gSystem->AccessPathName(tstr, kFileExists) == false);
      newfile = tstr;
    }
    gSystem->Rename(mFileName, newfile);
  }

  mLoggerThread->Kill(GThread::SigUSR1);
}

//------------------------------------------------------------------------------

void ZLog::tl_LogLoop(ZLog* log)
{
  log->LogLoop();
}

void ZLog::LogLoop()
{
  // Takes care of log rotation.
  //
  // Should we also have message queue for log entries?

  static const Exc_t _eh("ZLog::LogLoop ");

  GThread::UnblockSignal(GThread::SigUSR1);

  while (true)
  {
    GTime::SleepMiliSec(10*1000, true, false);

    if (gSystem->AccessPathName(mFileName, kFileExists) == true)
    {
      TString time(GTime::ApproximateTime().ToDateTimeLocal(false));
      GMutexHolder _lck(mLoggerCond);
      mStream << "******************** Logging rotated at " << time << " ********************" << endl;
      mStream.close();
      mStream.open(mFileName, ios_base::out | ios_base::app);
      if (mStream.fail())
      {
        ISerr(_eh + "Opening of log '" + mFileName + "' failed. Requesting logging termination.");
        mSaturn->ShootMIR(S_StopLogging());
      }
      mStream << "******************** Logging rotated at " << time << " ********************" << endl;
    }
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

void ZLog::Put(Int_t level, const TString& prefix, const TString& message)
{
  Put(GTime::ApproximateTime().ToDateTimeLocal(false), level, prefix, message);
}

void ZLog::Put(const GTime& time, Int_t level, const TString& prefix, const TString& message)
{
  Put(time.ToDateTimeLocal(false), level, prefix, message);
}

void ZLog::Put(const TString& time_string, Int_t level, const TString& prefix, const TString& message)
{
  LEVEL_CHECK(level);
  LEVEL_NAME(lvl_name, level);
  TString pim(prefix.EndsWith(" ") ? "" : " ");

  GMutexHolder _lck(mLoggerCond);
  mStream << time_string << " " << lvl_name << " " << prefix << pim << message << endl;
}

void ZLog::Form(Int_t level, const TString& prefix, const char* va_(fmt), ...)
{
  va_list ap;
  va_start(ap, va_(fmt));
  FormVA(GTime::ApproximateTime().ToDateTimeLocal(false), level, prefix, va_(fmt), ap);
  va_end(ap);
}

void ZLog::Form(const GTime& time, Int_t level, const TString& prefix, const char* va_(fmt), ...)
{
  va_list ap;
  va_start(ap, va_(fmt));
  FormVA(time.ToDateTimeLocal(false), level, prefix, va_(fmt), ap);
  va_end(ap);
}

void ZLog::Form(const TString& time_string, Int_t level, const TString& prefix, const char* va_(fmt), ...)
{
  LEVEL_CHECK(level);
  LEVEL_NAME(lvl_name, level);
  TString message;
  {
    va_list ap;
    va_start(ap, va_(fmt));
    message = GFormVA(va_(fmt), ap);
    va_end(ap);
  }
  TString pim(prefix.EndsWith(" ") ? "" : " ");

  GMutexHolder _lck(mLoggerCond);
  mStream << time_string << " " << lvl_name << " " << prefix << pim << message << endl;
}

void ZLog::FormVA(const GTime& time, Int_t level, const TString& prefix, const char* va_(fmt), va_list args)
{
  FormVA(time.ToDateTimeLocal(false), level, prefix, va_(fmt), args);
}

void ZLog::FormVA(const TString& time_string, Int_t level, const TString& prefix, const char* va_(fmt), va_list args)
{
  LEVEL_CHECK(level);
  LEVEL_NAME(lvl_name, level);
  TString message(GFormVA(va_(fmt), args));
  TString pim(prefix.EndsWith(" ") ? "" : " ");

  GMutexHolder _lck(mLoggerCond);
  mStream << time_string << " " << lvl_name << " " << prefix << pim << message << endl;
}


//==============================================================================
// ZLog::Helper
//==============================================================================

ZLog::Helper::Helper(ZLog* log, const TString& pfx) :
  m_log(log), m_prefix(pfx), m_level(L_Message)
{
  SetTime(GTime::ApproximateTime());
}

ZLog::Helper::Helper(ZLog* log, Int_t lvl, const TString& pfx) :
  m_log(log), m_prefix(pfx), m_level(lvl)
{
  SetTime(GTime::ApproximateTime());
}

ZLog::Helper::Helper(ZLog* log, const GTime& when, const TString& pfx) :
  m_log(log), m_prefix(pfx), m_level(L_Message)
{
  SetTime(when);
}

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
  if (m_log)
  {
    m_log->Put(m_time_string, m_level, m_prefix, message);
  }
}

void ZLog::Helper::Put(Int_t level, const TString& message)
{
  if (m_log)
  {
    m_log->Put(m_time_string, level, m_prefix, message);
  }
}

void ZLog::Helper::Form(const char* va_(fmt), ...)
{
  if (m_log)
  {
    va_list ap;
    va_start(ap, va_(fmt));
    m_log->FormVA(m_time_string, m_level, m_prefix, va_(fmt), ap);
    va_end(ap);
  }
}

void ZLog::Helper::Form(Int_t level, const char* va_(fmt), ...)
{
  if (m_log)
  {
    va_list ap;
    va_start(ap, va_(fmt));
    m_log->FormVA(m_time_string, level, m_prefix, va_(fmt), ap);
    va_end(ap);
  }
}
