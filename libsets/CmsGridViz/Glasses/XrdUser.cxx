// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdUser.h"
#include "XrdServer.h"
#include "XrdUser.c7"

// XrdUser

//______________________________________________________________________________
//
//

ClassImp(XrdUser);

//==============================================================================

void XrdUser::_init()
{}

XrdUser::XrdUser(const TString& n, const TString& t) :
  ZList(n, t)
{
  _init();
}

XrdUser::XrdUser(const TString& n, const TString& t, const TString& fh, const TString& fd, const GTime& st) :
  ZList(n, t),
  mFromHost(fh), mFromDomain(fd), mStartTime(st)
{
  _init();
}

XrdUser::~XrdUser()
{}

//==============================================================================
