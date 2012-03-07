// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdDomain.h"
#include "XrdDomain.c7"
#include "XrdServer.h"

// XrdDomain

//______________________________________________________________________________
//
//

ClassImp(XrdDomain);

//==============================================================================

void XrdDomain::_init()
{
  mPacketCount = mSeqIdFailCount = 0;
}

XrdDomain::XrdDomain(const Text_t* n, const Text_t* t) :
  ZNameMap(n, t)
{
  _init();
  SetElementFID(XrdServer::FID());
  SetKeepSorted(true);
}

XrdDomain::~XrdDomain()
{}

//==============================================================================

void XrdDomain::IncPacketCount()
{
  if (++mPacketCount % 100 == 0)
    Stamp(FID());
}

void XrdDomain::IncSeqIdFailCount()
{
  ++mSeqIdFailCount;
  Stamp(FID());
}
