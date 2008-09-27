// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRnrModBase_H
#define GledCore_ZRnrModBase_H

#include <Glasses/ZGlass.h>

class ZRnrModBase : public ZGlass
{
  MAC_RNR_FRIENDS(ZRnrModBase);

public:
  enum Operation_e { O_Nop=-1, O_Off=0, O_On = 1 };

private:
  void _init();

protected:

public:
  ZRnrModBase(const Text_t* n="ZRnrModBase", const Text_t* t=0) : ZGlass(n,t) { _init(); }


#include "ZRnrModBase.h7"
  ClassDef(ZRnrModBase, 1)
}; // endclass ZRnrModBase


#endif
