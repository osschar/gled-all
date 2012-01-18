// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_XrdMonSucker_H
#define CmsGridViz_XrdMonSucker_H

#include <Glasses/ZNameMap.h>
#include <Stones/SXrdServerId.h>
#include <Gled/GTime.h>

#include "TPRegexp.h"

class XrdServer;
class XrdUser;
class XrdFile;
class XrdFileCloseReporter;

class ZHashList;
class ZLog;


class XrdMonSucker : public ZNameMap
{
  MAC_RNR_FRIENDS(XrdMonSucker);

private:
  void _init();

protected:
  ZLink<ZLog>       mLog;       // X{GS} L{}
  ZLink<ZHashList>  mOpenFiles; // X{GS} L{}

  Int_t      mSuckPort;     // X{GS} 7 Value()

  TString    mNagiosUser;   // X{GS} 7 Textor()
  TString    mNagiosHost;   // X{GS} 7 Textor()
  TString    mNagiosDomain; // X{GS} 7 Textor()

  TString    mTraceDN;      // X{GS} Ray{TraceRE} 7 Textor()
  TString    mTraceHost;    // X{GS} Ray{TraceRE} 7 Textor()
  TString    mTraceDomain;  // X{GS} Ray{TraceRE} 7 Textor()

  ZLink<XrdFileCloseReporter> mFCReporter; // X{GS} L{}

  Int_t      mUserKeepSec;       // X{GS}   7 Value(-range=>[10, 86400, 1])
  Int_t      mUserDeadSec;       // X{GS}   7 Value(-range=>[300, 604800, 1])
  Int_t      mServDeadSec;       // X{GS}   7 Value(-range=>[300, 604800, 1])
  GTime      mLastOldUserCheck;  // X{GRSQ} 7 TimeOut()
  GTime      mLastDeadUserCheck; // X{GRSQ} 7 TimeOut()
  GTime      mLastDeadServCheck;     // X{GRSQ} 7 TimeOut()

  Int_t      mSocket;         //!
  GThread   *mSuckerThread;   //!
  GThread   *mCheckerThread;  //!

#ifndef __CINT__
  Bool_t     bTraceAllNull;   //!
  TPMERegexp mTraceDN_RE;     //!
  TPMERegexp mTraceHost_RE;   //!
  TPMERegexp mTraceDomain_RE; //!
#endif

#ifndef __CINT__
  typedef hash_map<SXrdServerId, XrdServer*, SXrdServerId::Hash> xrd_hash_t;
  typedef xrd_hash_t::iterator                                   xrd_hash_i;

  xrd_hash_t    m_xrd_servers;       //!
  GMutex        m_xrd_servers_mutex; //!
#endif

  void on_file_open(XrdFile* file);
  void on_file_close(XrdFile* file);

  void disconnect_user_and_close_open_files(XrdUser* user, XrdServer* server,
                                            const GTime& time);

  static void* tl_Suck(XrdMonSucker* s);
  void Suck();
  static void* tl_Check(XrdMonSucker* s);
  void Check();

public:
  XrdMonSucker(const Text_t* n="XrdMonSucker", const Text_t* t=0);
  virtual ~XrdMonSucker();

  virtual void AdEnlightenment();

  void StartSucker(); // X{Ed} 7 MButt()
  void StopSucker();  // X{Ed} 7 MButt()

  void CleanUpOldUsers();    // X{Ed} 7 MButt()
  void CleanUpDeadUsers();   // X{Ed} 7 MButt()
  void CleanUpDeadServers(); // X{Ed} 7 MButt()

  void EmitTraceRERay();

#include "XrdMonSucker.h7"
  ClassDef(XrdMonSucker, 1);
}; // endclass XrdMonSucker

#endif
