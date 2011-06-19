// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_CmsXrdHost_H
#define CmsGridViz_CmsXrdHost_H

#include <Glasses/ZNameMap.h>

class CmsXrdHost : public ZNameMap
{
  MAC_RNR_FRIENDS(CmsXrdHost);

private:
  void _init();

protected:

public:
  CmsXrdHost(const Text_t* n="CmsXrdHost", const Text_t* t=0);
  virtual ~CmsXrdHost();

#include "CmsXrdHost.h7"
  ClassDef(CmsXrdHost, 1);
}; // endclass CmsXrdHost

#endif
