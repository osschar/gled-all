// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_XrdServer_H
#define CmsGridViz_XrdServer_H

#include <Glasses/ZNameMap.h>
#include <Gled/GTime.h>

class XrdUser;
class XrdFile;

class XrdServer : public ZNameMap
{
  MAC_RNR_FRIENDS(XrdServer);

private:
  void _init();

protected:
  TString           mHost;        // X{GR}   7 TextOut()
  TString           mDomain;      // X{GR}   7 TextOut()
  GTime             mStartTime;   // X{GR}   7 TimeOut()
  GTime             mLastMsgTime; // X{GRSQ} 7 TimeOut()

  ZLink<ZList>      mPrevUsers;   // X{GS} L{}

  typedef map<Int_t, XrdUser*>    mDict2User_t;
  typedef mDict2User_t::iterator  mDict2User_i;
  typedef map<Int_t, XrdFile*>    mDict2File_t;
  typedef mDict2File_t::iterator  mDict2File_i;

  mDict2User_t      mUserMap; //!
  mDict2File_t      mFileMap; //!

  UChar_t           mLastSeq; //!

public:
  XrdServer(const TString& n="XrdServer", const TString& t="");
  XrdServer(const TString& n, const TString& t, const TString& h, const TString& d, const GTime& st);
  virtual ~XrdServer();

  virtual void AdEnlightenment();

  Bool_t   ExistsUserDictId(Int_t dict_id);
  void     AddUser(XrdUser* user, Int_t dict_id);
  void     DisconnectUser(XrdUser* user, Int_t dict_id);
  void     RemoveUser(XrdUser* user);
  XrdUser* FindUser(const TString& name);
  XrdUser* FindUser(Int_t dict_id);

  Bool_t   ExistsFileDictId(Int_t dict_id);
  void     AddFile(XrdFile* file, Int_t dict_id);
  void     RemoveFile(XrdFile* file, Int_t dict_id);
  XrdFile* FindFile(Int_t dict_id);

  // Only called from XrdMonSucker to initialize / check message sequence id.
  void    InitSrvSeq(UChar_t seq) { mLastSeq = seq;    }
  UChar_t IncAndGetSrvSeq()       { return ++mLastSeq; }

#include "XrdServer.h7"
  ClassDef(XrdServer, 1);
}; // endclass XrdServer

#endif
