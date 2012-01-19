// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZLog_H
#define GledCore_ZLog_H

#include <Glasses/ZGlass.h>

#include <Gled/GCondition.h>
#include <Gled/GTime.h>

#include <fstream>
#include <stdarg.h>

class GThread;


class ZLog : public ZGlass
{
  MAC_RNR_FRIENDS(ZLog);

public:
  enum Level_e
  {
    L_Fatal, L_Error, L_Warning, L_Message, L_Info, L_Debug
  };

  class Helper
  {
    ZLog    *m_log;
    GTime    m_time;
    TString  m_time_string;
    TString  m_prefix;
    Int_t    m_level;

  public:
    Helper(ZLog* log, const GTime& when, Int_t lvl, const TString& pfx="");

    void SetTime(const GTime& time);

    void Put(const TString& message);
    void Put(Int_t level, const TString& message);

    void Form(const char* fmt, ...);
    void Form(Int_t level, const char* fmt, ...);

    // ostream& operator<<();
  };

  struct Message
  {
    GTime   m_when;
    TString m_prefix;
    TString m_message;
    Int_t   m_level;
  };

private:
  void _init();

protected:
  TString           mFileName;     // X{GS} 7 Textor()
  Level_e           mLevel;        // X{GS} 7 PhonyEnum()
  Int_t             mDebugLevel;   // X{GS} 7 Value(-range=>[0, 9, 1])
  Bool_t            bLogActive;    // X{G}  7 BoolOut()

  GCondition        mLoggerCond;   //!
  GThread          *mLoggerThread; //!
  ofstream          mStream;       //!

  static void tl_LogLoop(ZLog* log);
  void   LogLoop();

public:
  ZLog(const Text_t* n="ZLog", const Text_t* t=0);
  virtual ~ZLog();

  void StartLogging(); // X{Ed} 7 MButt()
  void StopLogging();  // X{Ed} 7 MButt()

  void RotateLog();    // X{Ed} 7 MButt()

  // void ReopenFile();   // X{E}

  void Put(Int_t level, const TString& prefix, const TString& message);
  void Put(Int_t level, const GTime& time, const TString& prefix, const TString& message);
  void Put(Int_t level, const TString& time_string, const TString& prefix, const TString& message);

  void Form(Int_t level, const TString& prefix, const char* fmt, ...);
  void Form(Int_t level, const GTime& time, const TString& prefix, const char* fmt, ...);
  void Form(Int_t level, const TString& time_string, const TString& prefix, const char* fmt, ...);
  // #if defined(__GNUC__) && !defined(__CINT__)
  //    __attribute__((format(printf, 1, 2)))
  // #endif
  // ; // This is in root-tstring. Probably compile-time checks -- good!

  void Form(Int_t level, const GTime& time, const TString& prefix, const char* fmt, va_list args);
  void Form(Int_t level, const TString& time_string, const TString& prefix, const char* fmt, va_list args);

  // ostream& operator()(Int_t level);
  // ostream& operator()(Int_t level, const GTime& time);

#include "ZLog.h7"
  ClassDef(ZLog, 1);
}; // endclass ZLog

#endif
