// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZQueen_H
#define Gled_ZQueen_H

#include <Glasses/ZNameMap.h>
#include <Glasses/SaturnInfo.h>

class ZKing; class ZQueen;
class ZComet;
class Ray;

typedef set<ZQueen*>		spZQueen_t;
typedef set<ZQueen*>::iterator	spZQueen_i;

class ZQueen : public ZNameMap {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZQuuen);
  friend class ZKing;
  friend class Saturn;

public:
  //enum Contents_e { QC_Static = 0, QT_Dynamic = 1 };

  enum AuthMode_e { AM_None=0, AM_Queen, AM_Lens,
		    AM_QueenThenLens, AM_LensThenQueen };
  enum Align_e    { A_Good=0, A_Evil };

private:
  void _init();

protected:
  ZKing*	mKing;		  //! X{G}

  Bool_t	bMandatory;	  //  X{GS} 7 BoolOut(-join=>1)
  Bool_t	bRuling;	  //! X{GS} 7 BoolOut()
  Bool_t	bAwaitingSceptre; //! X{GS} 7 BoolOut()

  ID_t		mIDSpan;	// X{G} 7 ValOut()
  ID_t		mMinID;		// X{G} 7 ValOut(-range=>[0,MAX_ID,1,0], -width=>10, -join=>1)
  ID_t		mMaxID;		// X{G} 7 ValOut(-range=>[0,MAX_ID,1,0], -width=>10)
  ID_t		mMaxUsedID;	//!
  sID_t		mFreeIDs;	//!

  Bool_t	bStamping;	// X{GS} 7 Bool()

  UChar_t	mAuthMode;  // X{GS} 7 PhonyEnum(-seqvals=>[Null,Queen,Lens,QueenThenLens,LensThenQueen], -width=>10)
  UChar_t	mAlignment; // X{GS} 7 PhonyEnum(-seqvals=>[Good, Evil], -width=>6)
  UChar_t	mMapNoneTo; // X{GS} 7 PhonyEnum(-vals=>[ZMirFilter::R_Allow,Allow,ZMirFilter::R_Deny,Deny], -width=>8)

  ZMirFilter*	mProtector; // X{GS} L{}

  ZHashList*	mDeps;		//  X{GS} L{}
  ZHashList*	mOrphans;	//  X{GS} L{}

  lpSaturnInfo_t mReflectors;	//!

  virtual void Bootstrap();
  virtual void BlessMIR(ZMIR& mir) throw(string);

  virtual ZGlass* instantiate(LID_t lid, CID_t cid,
		    const Text_t* name=0, const Text_t* title=0); // X{E}

public:
  ZQueen(const Text_t* n="ZQueen", const Text_t* t=0) :
    ZNameMap(n,t), mIDSpan(0), mMinID(0), mMaxID(0), mMaxUsedID(0)
  { _init(); }
  ZQueen(ID_t span, const Text_t* n="ZQueen", const Text_t* t=0) :
    ZNameMap(n,t), mIDSpan(span), mMinID(0), mMaxID(0), mMaxUsedID(0)
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

  ID_t ReserveID();
  ID_t CheckIn(ZGlass* glass);
  void CheckOut(ZGlass* glass);
  //void CheckInWithAttach(ZGlass* glass);

  //void Adopt(ZGlass* glass);
  //void Adopt(ZList* target, ZStarInfo* starinfo); // X{E} C{2}

  ZGlass* InstantiateWAttach(ZGlass* attach_to, ZGlass* attach_gamma,
	     LID_t att_lid, CID_t att_cid, MID_t att_mid,
	     LID_t new_lid, CID_t new_cid,
	     const Text_t* name=0, const Text_t* title=0); //
  // X{E} C{2} T{ZQueen::Instantiate}

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

  // lens stamping
  void EmitRay(Ray& ray);

  // tmp
  void ListAll();		 // X{E} 7 MButt()

#include "ZQueen.h7"
  ClassDef(ZQueen, 1)
}; // endclass ZQueen

GlassIODef(ZQueen);

// Attempt at server exec + broadcast

#define CALL_AND_BROADCAST(_lens_, _method_, ...) \
  _lens_->_method_(__VA_ARGS__); \
  { auto_ptr<ZMIR> mir(_lens_->S_##_method_(__VA_ARGS__)); \
    mSaturn->markup_posted_mir(*mir); \
    mSaturn->BroadcastMIR(*mir, mReflectors); }

#endif
