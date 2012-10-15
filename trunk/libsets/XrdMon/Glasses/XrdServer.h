// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_XrdServer_H
#define XrdMon_XrdServer_H

#include <Glasses/ZNameMap.h>
#include <Stones/SXrdServerId.h>
#include <Gled/GTime.h>

class XrdMonSucker;
class XrdUser;
class XrdFile;


class XrdServer : public ZNameMap
{
  MAC_RNR_FRIENDS(XrdServer);
  friend class XrdMonSucker;

private:
  SXrdServerId m_server_id; //!

  void _init();

protected:
  TString           mHost;        // X{GR}   7 TextOut()
  TString           mDomain;      // X{GR}   7 TextOut()
  GTime             mStartTime;   // X{GR}   7 TimeOut()
  GTime             mLastMsgTime; // X{GRSQ} 7 TimeOut()

  // Add method to update these two values
  // + check in XrdMonSuck::CleanupOldServers()
  // + var saying how many deltas to wait before cleaning it up.
  // Have MaxDelta instead of MinDelta?
  GTime             mLastSrvIdTime;   //! X{GRSQ} 7 TimeOut()
  Int_t             mAvgSrvIdDelta;   //! X{G}    7 ValOut()

  Long64_t          mPacketCount;     //! X{G}    7 ValOut()
  Long64_t          mSeqIdFailCount;  //! X{G}    7 ValOut()

  ZLink<ZList>      mPrevUsers;   // X{GS} L{}

  typedef map<UInt_t, XrdUser*>   mDict2User_t;
  typedef mDict2User_t::iterator  mDict2User_i;
  typedef map<UInt_t, XrdFile*>   mDict2File_t;
  typedef mDict2File_t::iterator  mDict2File_i;

  mDict2User_t      mUserMap;      //!
  mDict2User_t      mPrevUserMap;  //!
  mDict2File_t      mFileMap;      //!
  GMutex            mUserMapMutex; //!
  GMutex            mFileMapMutex; //!

  UChar_t           mSrvSeq;       //!
  Bool_t            bSrvSeqInited; //!

public:
  XrdServer(const TString& n="XrdServer", const TString& t="");
  XrdServer(const TString& n, const TString& t, const TString& h, const TString& d, const GTime& st);
  virtual ~XrdServer();

  virtual void AdEnlightenment();

  TString  GetFqhn() const;

  void     UpdateSrvIdTime(const GTime& t);

  void     IncPacketCount();
  void     IncSeqIdFailCount();

  Bool_t   ExistsUserDictId(UInt_t dict_id);
  void     AddUser(XrdUser* user, UInt_t dict_id); // X{E} C{1}
  void     DisconnectUser(XrdUser* user);          // X{E} C{1}
  void     RemovePrevUser(XrdUser* user);          // X{E} C{1}
  Int_t    RemovePrevUsersOlderThan(const GTime& cut_time);
  void     ClearPrevUserMap();
  XrdUser* FindUser(const TString& name);
  XrdUser* FindUser(UInt_t dict_id);
  XrdUser* FindUserOrPrevUser(UInt_t dict_id);

  Bool_t   ExistsFileDictId(UInt_t dict_id);
  void     AddFile(XrdFile* file, UInt_t dict_id); // X{E} C{1}
  void     RemoveFile(XrdFile* file);              // X{E} C{1}
  XrdFile* FindFile(UInt_t dict_id);

  // Only called from XrdMonSucker to initialize / check message sequence id.
  void    InitSrvSeq(UChar_t seq) { mSrvSeq = seq; bSrvSeqInited = true; }
  UChar_t IncAndGetSrvSeq()       { return bSrvSeqInited ? ++mSrvSeq : 0; }
  Bool_t  IsSrvSeqInited() const  { return bSrvSeqInited; }
  void    ResetSrvSeq()           { mSrvSeq = 0; bSrvSeqInited = false; }

#include "XrdServer.h7"
  ClassDef(XrdServer, 1);
}; // endclass XrdServer

#endif
