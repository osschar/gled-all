// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZRlNameStack_H
#define GledCore_ZRlNameStack_H

#include <Glasses/ZRnrModBase.h>

class ZRlNameStack : public ZRnrModBase {
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(ZRlNameStack);

private:
  void _init();

protected:
  ZRnrModBase::Operation_e mNameStackOp;  // X{GS} 7 PhonyEnum()
  Bool_t                   bClearStack;   // X{GS} 7 Bool(-join=>1)
  Bool_t                   bRestoreStack; // X{GS} 7 Bool()

public:
  ZRlNameStack(const Text_t* n="ZRlNameStack", const Text_t* t=0) :
    ZRnrModBase(n,t) { _init(); }

#include "ZRlNameStack.h7"
  ClassDef(ZRlNameStack, 1)
}; // endclass ZRlNameStack


#endif
