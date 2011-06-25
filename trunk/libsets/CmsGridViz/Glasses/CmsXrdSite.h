// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_CmsXrdSite_H
#define CmsGridViz_CmsXrdSite_H

#include <Glasses/ZNameMap.h>

class CmsXrdSite : public ZNameMap
{
  MAC_RNR_FRIENDS(CmsXrdSite);

private:
  void _init();

protected:

public:
  CmsXrdSite(const Text_t* n="CmsXrdSite", const Text_t* t=0);
  virtual ~CmsXrdSite();

#include "CmsXrdSite.h7"
  ClassDef(CmsXrdSite, 1);
}; // endclass CmsXrdSite

#endif
