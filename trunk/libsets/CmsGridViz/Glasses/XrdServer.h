// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_XrdServer_H
#define CmsGridViz_XrdServer_H

#include <Glasses/ZNameMap.h>
#include <Gled/GTime.h>

class XrdServer : public ZNameMap
{
  MAC_RNR_FRIENDS(XrdServer);

private:
  void _init();

protected:
  TString           mHost;      // X{GR} 7 TextOut();
  TString           mDomain;    // X{GR} 7 TextOut();
  GTime             mStartTime; // X{GR} 7 TimeOut();

public:
  XrdServer(const TString& n="XrdServer", const TString& t="");
  XrdServer(const TString& n, const TString& t, const TString& h, const TString& d, const GTime& st);
  virtual ~XrdServer();

#include "XrdServer.h7"
  ClassDef(XrdServer, 1);
}; // endclass XrdServer

#endif
