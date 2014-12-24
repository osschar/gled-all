// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_SMessage_H
#define CmsGridViz_SMessage_H

#include "Gled/GledTypes.h"
#include "TMessage.h"

class ZLog;
class TSocket;

class SMessage : public TMessage
{
public:
  SMessage(UInt_t what=kMESS_ANY, Int_t bufsize=TBuffer::kInitialSize);
  SMessage(void* buf, Int_t bufsize);
  virtual ~SMessage();

  void SetLength() const { TMessage::SetLength(); }

  void    Send(TSocket* sock, Bool_t set_length);
  Bool_t  SendOrReport(TSocket* sock, Bool_t set_length, const Exc_t& eh,
                       Bool_t log_to_is=true, ZLog* log=0);

  static SMessage* Receive(TSocket* sock);
  static SMessage* ReceiveOrReport(TSocket* sock, const Exc_t& eh,
                                   Bool_t log_to_is=true, ZLog* log=0);

  ClassDef(SMessage, 0);
}; // endclass SMessage

#endif
