// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_SMessage_H
#define CmsGridViz_SMessage_H

#include "TMessage.h"

class SMessage : public TMessage
{
public:
  SMessage(UInt_t what=kMESS_ANY, Int_t bufsize=TBuffer::kInitialSize);
  SMessage(void* buf, Int_t bufsize);
  virtual ~SMessage();

  void SetLength() const { TMessage::SetLength(); }

  ClassDef(SMessage, 0);
}; // endclass SMessage

#endif
