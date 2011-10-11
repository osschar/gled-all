// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdUser.h"
#include "XrdServer.h"
#include "XrdUser.c7"
#include "XrdFile.h"

// XrdUser

//______________________________________________________________________________
//
// Represents a xrootd session on a given server.
// Can open several files, all stored as children.

ClassImp(XrdUser);

//==============================================================================

void XrdUser::_init()
{
  bTraceMon = false;
}

XrdUser::XrdUser(const TString& n, const TString& t) :
  ZList(n, t),
  mLoginTime(GTime::I_Never), mDisconnectTime(GTime::I_Never), mLastMsgTime(GTime::I_Never)
{
  _init();
}

XrdUser::XrdUser(const TString& n, const TString& t,
		 const TString& dn, const TString& vo, const TString& ro, const TString& gr,
                 const TString& su, const TString& fh, const TString& fd, const GTime& st) :
  ZList(n, t),
  mDN(dn), mVO(vo), mRole(ro), mGroup(gr),
  mServerUsername(su), mFromHost(fh), mFromDomain(fd),
  mLoginTime(st), mDisconnectTime(GTime::I_Never), mLastMsgTime(GTime::I_Never)

{
  _init();
}

XrdUser::~XrdUser()
{}

//==============================================================================

void XrdUser::AddFile(XrdFile* file)
{
  Add(file);
}
