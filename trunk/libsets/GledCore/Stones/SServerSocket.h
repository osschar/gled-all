// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SServerSocket_H
#define GledCore_SServerSocket_H

#include "SSocket.h"

class SServerSocket : public SSocket
{
private:
  SServerSocket();
  SServerSocket(const SServerSocket &);
  void operator=(const SServerSocket &);

public:
  enum { kDefaultBacklog = 10 };

  SServerSocket(Int_t port, Bool_t reuse = kFALSE, Int_t backlog = kDefaultBacklog,
		Int_t tcpwindowsize = -1);
  SServerSocket(const char *service, Bool_t reuse = kFALSE,
		Int_t backlog = kDefaultBacklog, Int_t tcpwindowsize = -1);
  virtual ~SServerSocket() {}

  virtual SSocket*      Accept();
  virtual TInetAddress  GetLocalInetAddress();
  virtual Int_t         GetLocalPort();

  ClassDef(SServerSocket, 0);
}; // endclass SServerSocket

#endif
