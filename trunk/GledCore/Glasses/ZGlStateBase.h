// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlStateBase_H
#define GledCore_ZGlStateBase_H

#include <Glasses/ZGlass.h>

class ZGlStateBase : public ZGlass {
  MAC_RNR_FRIENDS(ZGlStateBase);

 public:
  enum GlStateOp_e { GSO_Nop, GSO_On, GSO_Off };

private:
  void _init();

protected:

public:
  ZGlStateBase(const Text_t* n="ZGlStateBase", const Text_t* t=0) : ZGlass(n,t) { _init(); }


#include "ZGlStateBase.h7"
  ClassDef(ZGlStateBase, 1)
}; // endclass ZGlStateBase

GlassIODef(ZGlStateBase);

#endif
