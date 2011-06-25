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

XrdServer::XrdServer(const Text_t* n, const Text_t* t) :
  ZNameMap(n, t)
{
  _init();
}

XrdServer::XrdServer(const Text_t* n, const Text_t* t, const Text_t* h, const Text_t* d) :
  ZNameMap(n, t),
  mHost(h), mDomain(d)
{
  _init();
}

XrdServer::~XrdServer()
{}

//==============================================================================
