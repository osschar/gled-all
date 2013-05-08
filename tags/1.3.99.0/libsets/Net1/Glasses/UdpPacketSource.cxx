// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "UdpPacketSource.h"
#include "Glasses/ZLog.h"
#include "UdpPacketSource.c7"

// UdpPacketSource

//______________________________________________________________________________
//
//

ClassImp(UdpPacketSource);

//==============================================================================

void UdpPacketSource::_init()
{}

UdpPacketSource::UdpPacketSource(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

UdpPacketSource::~UdpPacketSource()
{}

//==============================================================================

void UdpPacketSource::RegisterConsumer(Queue_t* q)
{
  mConsumerSet.RegisterQueue(q);
}

void UdpPacketSource::UnregisterConsumer(Queue_t* q)
{
  mConsumerSet.UnregisterQueue(q);
}
