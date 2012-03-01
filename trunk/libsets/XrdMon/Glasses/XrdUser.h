// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_XrdUser_H
#define XrdMon_XrdUser_H

#include <Glasses/ZList.h>
#include <Gled/GTime.h>

class XrdServer;
class XrdFile;

class XrdUser : public ZList
{
  MAC_RNR_FRIENDS(XrdUser);
  friend class XrdServer;

private:
  Int_t             m_dict_id;       //!

  void _init();

protected:
  TString           mRealName;       // X{GR}   7 TextOut()
  TString           mDN;             // X{GR}   7 TextOut()
  TString           mVO;             // X{GR}   7 TextOut()
  TString           mRole;           // X{GR}   7 TextOut()
  TString           mGroup;          // X{GR}   7 TextOut()
  TString           mServerUsername; // X{GR}   7 TextOut()
  TString           mFromHost;       // X{GR}   7 TextOut()
  TString           mFromDomain;     // X{GR}   7 TextOut()
  TString           mAppInfo;        // X{GR}   7 TextOut()
  GTime             mLoginTime;      // X{GR}   7 TimeOut()
  GTime             mDisconnectTime; // X{GRSQ} 7 TimeOut()
  GTime             mLastMsgTime;    // X{GRSQ} 7 TimeOut()

  Bool_t            bTraceMon;       //!X{GS}  7 Bool()

  ZLink<XrdServer>  mServer;         // X{GS} L{}

public:
  XrdUser(const TString& n="XrdUser", const TString& t="");
  XrdUser(const TString& n, const TString& t, 
	  const TString& dn, const TString& vo, const TString& ro, const TString& gr,
          const TString& su, const TString& fh, const TString& fd, const GTime& st);
  virtual ~XrdUser();

  Bool_t IsConnected() const { return mDisconnectTime.IsNever() && ! mLoginTime.IsNever(); }

  void AddFile(XrdFile* file);

  void AppendAppInfo(const TString& app_info);

  static TString    ParseHumanName(const TString& dn);

#include "XrdUser.h7"
  ClassDef(XrdUser, 1);
}; // endclass XrdUser

#endif
