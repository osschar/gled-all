// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef XrdMon_XrdDomain_H
#define XrdMon_XrdDomain_H

#include <Glasses/ZNameMap.h>

class XrdDomain : public ZNameMap
{
  MAC_RNR_FRIENDS(XrdDomain);

private:
  void _init();

protected:
  Long64_t          mPacketCount;       //!X{G}    7 ValOut()
  Long64_t          mSeqIdFailCount;    //!X{G}    7 ValOut()

public:
  XrdDomain(const Text_t* n="XrdDomain", const Text_t* t=0);
  virtual ~XrdDomain();

  void     IncPacketCount();
  void     IncSeqIdFailCount();

#include "XrdDomain.h7"
  ClassDef(XrdDomain, 1);
}; // endclass XrdDomain

#endif
