// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZKing_H
#define Gled_ZKing_H

#include <Glasses/ZHashList.h>
#include <Glasses/SaturnInfo.h>
class ZQueen;

class ZKing : public ZHashList {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZKing);
  friend class Saturn;

private:
  void _init();

protected:
  SaturnInfo*	mSaturnInfo;	// X{GS} L{} Saturn that shines for this King
  ID_t		mMinID;		// X{G} ValOut(-range=>[0,MAX_ID,1,0], -width=>10, -join=>1)
  ID_t		mMaxID;		// X{G} ValOut(-range=>[0,MAX_ID,1,0], -width=>10)
  ID_t		mMaxUsedID;	//

  Bool_t	bFireKing;	// X{G} BoolOut()

  virtual void BlessMIR(ZMIR& mir) throw(string);

public:
  ZKing(const Text_t* n="ZKing", const Text_t* t=0) :
    ZHashList(n,t), mMinID(0), mMaxID(0), mMaxUsedID(0) { _init(); }
  ZKing(ID_t min, ID_t max, const Text_t* n="ZKing", const Text_t* t=0) :
    ZHashList(n,t), mMinID(min), mMaxID(max), mMaxUsedID(min-1) { _init(); }

  void Enthrone(ZQueen* queen);

  void StarToQueen(ZComet* comet, ID_t span);

  ZComet* MakeComet();

  // Dependency of Queens
  //void AddDependency(ZQueen* target, ZQueen* new_dep); // X{E} C{2}
  //void RemoveDependency(ZQueen* target, ZQueen* dep);  // X{E} C{2}

  // Mirroring of queens. FireKing interface (should have FireKing glass).
  void ReflectQueen(ZQueen* queen_to_mirror);	// X{E} C{1}
  void UnreflectQueen(ZQueen* queen_to_leave);	// X{E} C{1}

  // The following ones are internal.
  // Should upgrade p7: Ctx{#q}; q ~ do not export to ContextMethodList
  void reflect_queen(ZQueen* queen_to_mirror, SaturnInfo* moon);  // X{E} C{2q}
  void activate_queen(ZQueen* queen);				  // X{E} C{1q}
  void unreflect_queen(ZQueen* queen_to_leave, SaturnInfo* moon); // X{E} C{2q}

#include "ZKing.h7"
  ClassDef(ZKing, 1) // Ruler of top-level object-spaces
}; // endclass ZKing

GlassIODef(ZKing);

#endif
