// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdUser.h"
#include "XrdServer.h"
#include "XrdUser.c7"
#include "XrdFile.h"
#include "TPRegexp.h"

// XrdUser

//______________________________________________________________________________
//
// Represents a xrootd session on a given server.
// Can open several files, all stored as children.

ClassImp(XrdUser);

//==============================================================================

void XrdUser::_init()
{
  m_dict_id = 0;
}

XrdUser::XrdUser(const TString& n, const TString& t) :
  ZList(n, t),
  bNumericHost(false),
  mLoginTime(GTime::I_Never), mDisconnectTime(GTime::I_Never), mLastMsgTime(GTime::I_Never)
{
  _init();
}

XrdUser::XrdUser(const TString& n, const TString& t,
		 const TString& dn, const TString& vo, const TString& ro, const TString& gr,
                 const TString& su, const TString& fh, const TString& fd, Bool_t num_host,
                 const GTime& st) :
  ZList(n, t),
  mDN(dn), mVO(vo), mRole(ro), mGroup(gr),
  mServerUsername(su), mFromHost(fh), mFromDomain(fd), bNumericHost(num_host),
  mLoginTime(st), mDisconnectTime(GTime::I_Never), mLastMsgTime(GTime::I_Never)

{
  _init();
  mRealName = ParseHumanName(mDN);
}

XrdUser::~XrdUser()
{}

//------------------------------------------------------------------------------

TString XrdUser::GetFromFqhn() const
{
  if (bNumericHost)
    return mFromHost + "@" + mFromDomain; // What? Should know if it's local
  else
    return mFromHost + "." + mFromDomain;
}

//==============================================================================

void XrdUser::AddFile(XrdFile* file)
{
  Add(file);
}

void XrdUser::AppendAppInfo(const TString& app_info)
{
  mAppInfo += app_info;
  Stamp(FID());
}

//==============================================================================

TString XrdUser::ParseHumanName(const TString& dn)
{
  static TPMERegexp re("/CN=(.*?)(?=(?:/CN=)|$)", "g");
  static TPMERegexp numex("(.*) (\\d+)");
  static TPMERegexp mail1("(.*) \\(?[\\w\\.]+@[\\w\\.]+\\(?");
  static TPMERegexp mail2("(.*)/Email=[\\w\\.]+@[\\w\\.]+");

  static GMutex re_mutex;
  GMutexHolder _lck(re_mutex);

  TString user;
  while (re.Match(dn))
  {
    if (re.NMatches() == 2)
    {
      // keep last matched user, except if it starts with UID 
      if (strncmp( "UID:", re[1].Data(), 4))
        user =  re[1].Data();
    } 
    else 
    {
      // set value to CN if mach fails
      user = dn;
    }
  }

  // remove numeric values from CN
  if (numex.Match(user))
    user = numex[1].Data();

  // remove email addresses
  if (mail1.Match(user))
    user = mail1[1].Data();
  else if (mail2.Match(user)) 
    user = mail2[1].Data();

  return user;
}
