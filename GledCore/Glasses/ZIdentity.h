// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZIdentity_H
#define GledCore_ZIdentity_H

#include <Glasses/ZGlass.h>

class ZIdentity : public ZGlass {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZIdentity);

  friend class ZSunQueen;

private:
  void _init();

protected:
  UInt_t		mNumMMEs;	// X{GS} 7 ValOut()
  ZMirFilter*		mAllowThis;	// X{GS} L{}

public:
  ZIdentity(const Text_t* n="ZIdentity", const Text_t* t=0) : ZGlass(n,t) { _init(); }

  void IncNumMMEs() {
    mExecMutex.Lock(); ++mNumMMEs; Stamp(LibID(), ClassID()); mExecMutex.Unlock();
  } // X{E}
  void DecNumMMEs() {
    mExecMutex.Lock(); --mNumMMEs; Stamp(LibID(), ClassID()); mExecMutex.Unlock();
  } // X{E}

#include "ZIdentity.h7"
  ClassDef(ZIdentity, 1)
}; // endclass ZIdentity

GlassIODef(ZIdentity);

#endif
