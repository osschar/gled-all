// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZKing_H
#define GledCore_ZKing_H

#include <Glasses/ZHashList.h>
#include <Glasses/SaturnInfo.h>
#include <Glasses/ZMirFilter.h>
class ZQueen;

class ZKing : public ZHashList
{
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZKing);

  friend class Saturn;

public:
  enum LightType_e { LT_Undef, LT_Moon, LT_Sun, LT_Fire };

private:
  void _init();

protected:
  SaturnInfo*	mSaturnInfo;	// X{GS} L{} Saturn that shines for this King
  ID_t		mMinID;		// X{G} ValOut(-range=>[0,MAX_ID,1,0], -width=>10, -join=>1)
  ID_t		mMaxID;		// X{G} ValOut(-range=>[0,MAX_ID,1,0], -width=>10)
  ID_t		mMaxUsedID;	//

  LightType_e	mLightType;	// X{G}

  UChar_t	mMapNoneTo;     // X{GS} 7 PhonyEnum(-type=>ZMirFilter::Result_e, -names=>[R_Allow,R_Deny], -width=>8)

  virtual void BlessMIR(ZMIR& mir) throw(string);

public:
  ZKing(const Text_t* n="ZKing", const Text_t* t=0) :
    ZHashList(n,t), mMinID(0), mMaxID(0), mMaxUsedID(0) { _init(); }
  ZKing(ID_t min, ID_t max, const Text_t* n="ZKing", const Text_t* t=0) :
    ZHashList(n,t), mMinID(min), mMaxID(max), mMaxUsedID(min-1) { _init(); }

  virtual ~ZKing();

  virtual void AdEnlightenment();

  virtual void Enthrone(ZQueen* queen);

  void StarToQueen(ZComet* comet, ID_t span);

  ZComet* MakeComet();

  // Dependency of Queens
  //void AddDependency(ZQueen* target, ZQueen* new_dep); // X{E} C{2}
  //void RemoveDependency(ZQueen* target, ZQueen* dep);  // X{E} C{2}

  // The following ones are internal.
  // Should upgrade p7: Ctx{#q}; q ~ do not export to ContextMethodList
  void reflect_queen(ZQueen* queen_to_mirror, SaturnInfo* moon);  // X{E} C{2q}
  void activate_queen(ZQueen* queen);				  // X{E} C{1q}
  void unreflect_queen(ZQueen* queen_to_leave, SaturnInfo* moon); // X{E} C{2q}

  void receive_eunuch(); // X{E}

  // A more general queen mirroring infrastructure; server push
  // void reflect_queen_to_saturn(ZQueen* queen_to_mirror, SaturnInfo* moon); // X{E} C{2}
  // void reflect_queen_to_all(ZQueen* queen);               // X{E} C{1}
  // void reflect_queen_to_list(ZQueen* queen, ZList* list); // X{E} C{2}

#include "ZKing.h7"
  ClassDef(ZKing, 1) // Ruler of top-level object-spaces
}; // endclass ZKing

GlassIODef(ZKing);

#endif
