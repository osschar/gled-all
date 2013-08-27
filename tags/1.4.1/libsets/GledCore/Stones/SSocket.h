// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SSocket_H
#define GledCore_SSocket_H

#include "Gled/GMutex.h"
#include "TSocket.h"

class SSocket : public TSocket
{
  friend class SServerSocket;

protected:
  GMutex   mMutex;
  Bool_t   mClosedDown;

public:
  SSocket() :
    TSocket(), mClosedDown(false) {}
  SSocket(TInetAddress address, const char *service, Int_t tcpwindowsize = -1) :
    TSocket(address, service, tcpwindowsize), mClosedDown(false) {}
  SSocket(TInetAddress address, Int_t port, Int_t tcpwindowsize = -1) :
    TSocket(address, port, tcpwindowsize), mClosedDown(false) {}
  SSocket(const char *host, const char *service, Int_t tcpwindowsize = -1) :
    TSocket(host, service, tcpwindowsize), mClosedDown(false) {}
  SSocket(const char *host, Int_t port, Int_t tcpwindowsize = -1) :
    TSocket(host, port, tcpwindowsize), mClosedDown(false) {}
  SSocket(const char *sockpath) :
    TSocket(sockpath), mClosedDown(false) {}
  SSocket(Int_t descriptor) :
    TSocket(descriptor), mClosedDown(false) {}
  SSocket(Int_t descriptor, const char *sockpath) :
    TSocket(descriptor, sockpath), mClosedDown(false) {}
  SSocket(const SSocket &s) :
    TSocket(s), mClosedDown(s.mClosedDown) {}
  virtual ~SSocket() {}

  virtual void Close(Option_t *opt="");

  ClassDef(SSocket, 0);
}; // endclass SSocket

#endif
