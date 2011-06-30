// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_XrdUser_H
#define CmsGridViz_XrdUser_H

#include <Glasses/ZList.h>
#include <Gled/GTime.h>

class XrdServer;

class XrdUser : public ZList
{
  MAC_RNR_FRIENDS(XrdUser);

private:
  void _init();

protected:
  TString           mFromHost;    // X{GR} 7 TextOut();
  TString           mFromDomain;  // X{GR} 7 TextOut();
  GTime             mStartTime;   // X{GR} 7 TimeOut();

  ZLink<XrdServer>  mServer;      // X{GS} L{}

public:
  XrdUser(const TString& n="XrdUser", const TString& t="");
  XrdUser(const TString& n, const TString& t, const TString& fh, const TString& fd, const GTime& st);
  virtual ~XrdUser();

#include "XrdUser.h7"
  ClassDef(XrdUser, 1);
}; // endclass XrdUser

#endif
