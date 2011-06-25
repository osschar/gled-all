// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef CmsGridViz_CmsXrdUser_H
#define CmsGridViz_CmsXrdUser_H

#include <Glasses/ZList.h>

class CmsXrdUser : public ZList
{
  MAC_RNR_FRIENDS(CmsXrdUser);

private:
  void _init();

protected:

public:
  CmsXrdUser(const Text_t* n="CmsXrdUser", const Text_t* t=0);
  virtual ~CmsXrdUser();

#include "CmsXrdUser.h7"
  ClassDef(CmsXrdUser, 1);
}; // endclass CmsXrdUser

#endif
