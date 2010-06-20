// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TabletStrokeList_H
#define Tmp1_TabletStrokeList_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>

class TabletStrokeList : public ZNode
{
  MAC_RNR_FRIENDS(TabletStrokeList);

private:
  void _init();

protected:
  ZColor               mColor;     // X{PRGS} 7 ColorButt()

public:
  TabletStrokeList(const Text_t* n="TabletStrokeList", const Text_t* t=0);
  virtual ~TabletStrokeList();

#include "TabletStrokeList.h7"
  ClassDef(TabletStrokeList, 1);
}; // endclass TabletStrokeList

#endif
