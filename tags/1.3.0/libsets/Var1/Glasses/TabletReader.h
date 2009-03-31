// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TabletReader_H
#define Var1_TabletReader_H

#include <Glasses/ZGlass.h>

class TabletReader : public ZGlass
{
  MAC_RNR_FRIENDS(TabletReader);

private:
  void _init();

protected:

public:
  TabletReader(const Text_t* n="TabletReader", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }


#include "TabletReader.h7"
  ClassDef(TabletReader, 1);
}; // endclass TabletReader


#endif
