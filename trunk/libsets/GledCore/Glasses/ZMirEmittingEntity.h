// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZMirEmittingEntity_H
#define GledCore_ZMirEmittingEntity_H

#include <Glasses/ZGlass.h>
#include <Glasses/ZIdentity.h>
#include <Glasses/ZHashList.h>

class SaturnInfo;

class ZMirEmittingEntity : public ZGlass
{
  MAC_RNR_FRIENDS(ZMirEmittingEntity);

  friend class Gled; friend class Saturn; friend class ZSunQueen;

private:
  void _init();

protected:
  TString           mLogin;             // X{GS} 7 TextOut()
  ZLink<ZIdentity>  mPrimaryIdentity;   // X{gS} L{}
  ZLink<ZHashList>  mActiveIdentities;  // X{gS} L{}

public:
  ZMirEmittingEntity(const Text_t* n="ZMirEmittingEntity", const Text_t* t=0) :
    ZGlass(n,t) { _init(); }

  virtual void AdEnlightenment();

  virtual SaturnInfo* HostingSaturn() = 0;
  virtual void Message(const TString& s) {} // X{E}
  virtual void Warning(const TString& s) {} // X{E} T{MEE::Self}
  virtual void Error  (const TString& s) {} // X{E} T{MEE::Self}

  Bool_t HasIdentity(ZIdentity* ident);

#include "ZMirEmittingEntity.h7"
  ClassDef(ZMirEmittingEntity, 1);
}; // endclass ZMirEmittingEntity


#endif
