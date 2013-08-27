// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_XrdEhs_H
#define XrdMon_XrdEhs_H

#include <Glasses/ZNameMap.h>
#include <Gled/GTime.h>

class XrdMonSucker;
class XrdFile;

#include "TPRegexp.h"

class SSocket;


class XrdEhs : public ZNameMap
{
private:
  void _init();

  Bool_t	       b_stop_server; //!

  GMutex               m_re_mutex;    //!
  TPMERegexp           m_req_line_re; //!
  TPMERegexp           m_req_re;      //!

protected:
  ZLink<XrdMonSucker>  mXrdSucker;    // X{GS} L{a}
  Int_t	               mPort;         // X{GS} 7 Value(-range=>[1,65535,1])
  Bool_t	       bServerUp;     // X{GS} 7 BoolOut()

  Bool_t               bParanoia;     // X{GS} 7 Bool()
  TString              mWebTableJs;   // X{GS} 7 Textor()

  list<XrdFile*>       mFileList;     //!
  TimeStamp_t          mFileListTS;   //!
  GMutex               mServeMutex;   //!

  void fill_content(const GTime& req_time, TString& content, lStr_t& path, mStr2Str_t& args);

public:
  XrdEhs(const Text_t* n="XrdEhs", const Text_t* t=0);
  virtual ~XrdEhs();

  void StartServer(); // X{Ed} 7 MButt(-join=>1)
  void StopServer();  // X{E}  7 MButt()

  void ServePage(SSocket* sock);

#include "XrdEhs.h7"
  ClassDef(XrdEhs, 1);
}; // endclass XrdEhs

#endif
