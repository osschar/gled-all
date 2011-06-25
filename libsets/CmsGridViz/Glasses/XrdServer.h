// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_XrdServer_H
#define CmsGridViz_XrdServer_H

#include <Glasses/ZNameMap.h>

class XrdServer : public ZNameMap
{
  MAC_RNR_FRIENDS(XrdServer);

private:
  void _init();

protected:
  TString           mHost;    // X{GR} 7 TextOut();
  TString           mDomain;  // X{GR} 7 TextOut();

public:
  XrdServer(const Text_t* n="XrdServer", const Text_t* t=0);
  XrdServer(const Text_t* n, const Text_t* t, const Text_t* h, const Text_t* d);
  virtual ~XrdServer();

#include "XrdServer.h7"
  ClassDef(XrdServer, 1);
}; // endclass XrdServer

#endif
