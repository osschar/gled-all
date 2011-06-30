// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdServer.h"
#include "XrdServer.c7"

// XrdServer

//______________________________________________________________________________
//
//

ClassImp(XrdServer);

//==============================================================================

void XrdServer::_init()
{}

XrdServer::XrdServer(const TString& n, const TString& t) :
  ZNameMap(n, t)
{
  _init();
}

XrdServer::XrdServer(const TString& n, const TString& t, const TString& h, const TString& d, const GTime& st) :
  ZNameMap(n, t),
  mHost(h), mDomain(d), mStartTime(st)
{
  _init();
}

XrdServer::~XrdServer()
{}

//==============================================================================
