// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZQueen_H
#define Gled_ZQueen_H

#include <Glasses/ZHashList.h>
#include <Glasses/SaturnInfo.h>

class ZKing; class ZQueen;
class ZComet;

typedef set<ZQueen*>		spZQueen_t;
typedef set<ZQueen*>::iterator	spZQueen_i;

class ZQueen : public ZHashList {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZQuuen);
  friend class ZKing;
  friend class Saturn;
public:
  //enum Contents_e { QC_Static = 0, QT_Dynamic = 1 };

private:
  void _init();

protected:
  ZKing*	mKing;		//! X{G}

  Bool_t	bMandatory;	  //  X{GS} 7 BoolOut(-join=>1)
  Bool_t	bRuling;	  //! X{GS} 7 BoolOut(-join=>1)
  Bool_t	bAwaitingSceptre; //! X{GS} 7 BoolOut()

  ID_t		mIDSpan;	// X{G} ValOut()
  ID_t		mMinID;		// X{G} ValOut(-range=>[0,MAX_ID,1,0], -width=>10, -join=>1)
  ID_t		mMaxID;		// X{G} ValOut(-range=>[0,MAX_ID,1,0], -width=>10)
  ID_t		mMaxUsedID;	//!
  sID_t		mFreeIDs;	//!

  ZHashList*	mDeps;		//  X{GS} L{l}
  ZHashList*	mOrphans;	//  X{GS} L{l}

  lpSaturnInfo_t mReflectors;	//!
  
  virtual void Bootstrap();
  virtual void BlessMIR(ZMIR& mir) throw(string);

public:
  ZQueen(const Text_t* n="ZQueen", const Text_t* t=0) :
    ZHashList(n,t), mIDSpan(0), mMinID(0), mMaxID(0), mMaxUsedID(0)
  { _init(); }
  ZQueen(ID_t span, const Text_t* n="ZQueen", const Text_t* t=0) :
    ZHashList(n,t), mIDSpan(span), mMinID(0), mMaxID(0), mMaxUsedID(0)
  { _init(); }

  // Dependencies
  //void AddDependency(ZQueen* new_dep); // X{E} C{1}
  //void RemoveDependency(ZQueen* dep); // X{E} C{1}
  // Reflectors
  //void AddLocReflector(SaturnInfo* moon); // X{E} C{1}
  //void RemoveLocReflector(SaturnInfo* moon); // X{E} C{1}
  // Orphans
  void ZeroRefCount(ZGlass* glass);
  void CleanOrphanage(); 		// X{E} 7 MButt()

  void CheckIn(ZGlass* glass) throw(string);
  void CheckOut(ZGlass* glass);
  //void CheckInWithAttach(ZGlass* glass);

  //void Adopt(ZGlass* glass);
  //void Adopt(ZList* target, ZStarInfo* starinfo); // X{E} C{2}

  ZGlass* InstantiateWAttach(ZGlass* attach_to, ZGlass* attach_gamma,
	     UInt_t lid, UInt_t cid,
	     const Text_t* name=0, const Text_t* title=0); // X{E} C{2IIpp}
  ZGlass* IncarnateWAttach(ZGlass* attach_to, ZGlass* attach_gamma); // X{E} C{2}
  // instantiate URL

  // Reflections ... operated by Kings
  virtual void CreateReflection(TBuffer& buf);
  virtual void InvokeReflection(TBuffer& buf);
  virtual void RevokeReflection();

  // Comet operations
  ZComet* MakeComet();
  void Adopt(ZComet* comet);
  virtual void UnfoldFrom(ZComet& comet);

  // reflector handling
  void add_reflector(SaturnInfo* moon);
  void remove_reflector(SaturnInfo* moon);

  // tmp
  void ListAll();		 // X{E} 7 MButt()

#include "ZQueen.h7"
  ClassDef(ZQueen, 1)
}; // endclass ZQueen

GlassIODef(ZQueen);

#endif
