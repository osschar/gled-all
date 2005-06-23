// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_EyeInfo_H
#define Gled_EyeInfo_H

// Includes
#include <Glasses/ZMirEmittingEntity.h>
#include <Glasses/SaturnInfo.h>

class Saturn;
class Eye;
class TSocket;

class EyeInfo : public ZMirEmittingEntity {
  MAC_RNR_FRIENDS(EyeInfo);
  friend class ZKing; friend class ZQueen; friend class ZSunQueen;
  friend class Saturn;

public:
  typedef Eye*	(*EyeCreator_foo)(TSocket*, EyeInfo*, ZGlass*);

private:
  TSocket*	hSocket;	//!

protected:
  SaturnInfo*	mMaster;		// X{gS} L{}

public:
  EyeInfo(const Text_t* n="EyeInfo", const Text_t* t=0);

  virtual SaturnInfo* HostingSaturn() { return mMaster; }
  // Virtuals exported from MEE
  virtual void Message(const Text_t* s);
  virtual void Warning(const Text_t* s);
  virtual void Error(const Text_t* s);

#include "EyeInfo.h7"
  ClassDef(EyeInfo,1)
}; // endclass EyeInfo

GlassIODef(EyeInfo);

typedef list<EyeInfo*>			lpEyeInfo_t;
typedef list<EyeInfo*>::iterator	lpEyeInfo_i;

#endif
