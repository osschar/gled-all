// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVSiteList_H
#define AliEnViz_AEVSiteList_H

#include <Glasses/ZNameMap.h>

class AEVSite;

class AEVSiteList : public ZNameMap
{
  MAC_RNR_FRIENDS(AEVSiteList);

private:
  void _init();

protected:

public:
  AEVSiteList(const Text_t* n="AEVSiteList", const Text_t* t=0);
  virtual ~AEVSiteList();

  AEVSite* FindSite(const TString& name);

#include "AEVSiteList.h7"
  ClassDef(AEVSiteList, 1);
}; // endclass AEVSiteList

#endif
