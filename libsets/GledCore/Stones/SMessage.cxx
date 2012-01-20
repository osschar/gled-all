// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SMessage.h"
#include "Glasses/ZLog.h"

#include "TSocket.h"

#include <cerrno>

// SMessage

//______________________________________________________________________________
//
// Wrapper around TMessage allowing somewhat more optimized use.

ClassImp(SMessage);

//==============================================================================

SMessage::SMessage(UInt_t what, Int_t bufsize) :
  TMessage(what, bufsize)
{}

SMessage::SMessage(void* buf, Int_t bufsize) :
  TMessage(buf, bufsize)
{}

SMessage::~SMessage()
{}

//------------------------------------------------------------------------------

void SMessage::Send(TSocket* sock, Bool_t set_length)
{
  if (set_length)
    SetLength();
  Int_t n = sock->SendRaw(Buffer(), Length());
  if (n != Length())
    throw n;
}

Bool_t SMessage::SendOrReport(TSocket* sock, Bool_t set_length, const Exc_t& eh,
                              Bool_t log_to_is, ZLog* log)
{
  if (set_length)
    SetLength();
  Int_t n = sock->SendRaw(Buffer(), Length());
  if (n != Length())
  {
    TString msg;
    if (n == 0)
    {
      msg.Form("Client %s closed connection.", sock->GetInetAddress().GetHostName());
      if (log_to_is) ISmess(eh + msg);
      if (log)       log->Put(ZLog::L_Message, eh, msg);
    }
    else
    {
      msg.Form("Error %d receiving from %s: %s.", n, sock->GetInetAddress().GetHostName(),
               strerror(errno));
      if (log_to_is) ISerr(eh + msg);
      if (log)       log->Put(ZLog::L_Error, eh, msg);
    }
    return false;
  }
  else
  {
    return true;
  }
}

//------------------------------------------------------------------------------

SMessage* SMessage::Receive(TSocket* sock)
{
  Int_t n, len_net;
  n = sock->RecvRaw(&len_net, sizeof(Int_t));
  if (n != sizeof(Int_t))
  {
    throw n;
  }
  Int_t len  = net2host(len_net);
  Int_t len2 = len + sizeof(Int_t);
  char  *buf = new char[len2];
  memcpy(buf, &len_net, sizeof(Int_t));
  n = sock->RecvRaw(buf + sizeof(Int_t), len);
  if (n != len)
  {
    delete [] buf;
    throw n;
  }
  return new SMessage(buf, len2);
}

SMessage* SMessage::ReceiveOrReport(TSocket* sock, const Exc_t& eh,
                                    Bool_t log_to_is, ZLog* log)
{
  SMessage *m = 0;
  try
  {
    m = Receive(sock);
  }
  catch (Int_t n)
  {
    TString msg;
    if (n == 0)
    {
      msg.Form("Client %s closed connection.", sock->GetInetAddress().GetHostName());
      if (log_to_is) ISmess(eh + msg);
      if (log)       log->Put(ZLog::L_Message, eh, msg);
    }
    else
    {
      msg.Form("Error %d receiving from %s: %s.", n, sock->GetInetAddress().GetHostName(),
               strerror(errno));
      if (log_to_is) ISerr(eh + msg);
      if (log)       log->Put(ZLog::L_Error, eh, msg);
    }
  }
  return m;
}
