// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZSunQueen_H
#define Gled_ZSunQueen_H

#include <Glasses/ZQueen.h>
#include <Glasses/SaturnInfo.h>

class ZSunQueen : public ZQueen {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZSunQueen);
  friend class ZKing;
  friend class Saturn;

private:
  void _init();

protected:
  SaturnInfo*		mSunInfo; // X{GS} L{}

public:
  ZSunQueen(const Text_t* n="ZSunQueen", const Text_t* t=0) : ZQueen(n,t)
  { _init(); }
  ZSunQueen(ID_t span, const Text_t* n="ZSunQueen", const Text_t* t=0) :
    ZQueen(span,n,t) { _init(); }

  virtual void UnfoldFrom(ZComet& comet);

  virtual void IncarnateMoon(SaturnInfo* parent); // X{E} C{1}
  virtual void IncarnateEye(SaturnInfo* parent);  // X{E} C{1}
  virtual void CremateMoon(SaturnInfo* moon);     // X{E} C{1}
  virtual void CremateEye(EyeInfo* eye);	  // X{E} C{1}

#include "ZSunQueen.h7"
  ClassDef(ZSunQueen, 1)
}; // endclass ZSunQueen

GlassIODef(ZSunQueen);

#endif
