// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_EyeInfo_H
#define Gled_EyeInfo_H

// Includes
#include <Glasses/ZGlass.h>
#include <Glasses/SaturnInfo.h>

class Saturn;

class TSocket;

class EyeInfo : public ZGlass {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0)")
  MAC_RNR_FRIENDS(EyeInfo);
  friend class ZKing; friend class ZQueen; friend class ZSunQueen;
  friend class Saturn;
private:
  TSocket*	hSocket;	//!
protected:
  SaturnInfo*	mMaster;	// X{GS} L{}

public:
  EyeInfo(const Text_t* n="EyeInfo", const Text_t* t=0);

  void Message(const Text_t* s); // X{E}
  void Error(const Text_t* s);   // X{E}

#include "EyeInfo.h7"
  ClassDef(EyeInfo,1)
}; // endclass EyeInfo

GlassIODef(EyeInfo);

typedef list<EyeInfo*>			lpEyeInfo_t;
typedef list<EyeInfo*>::iterator	lpEyeInfo_i;

#endif
