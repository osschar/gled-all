// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZQueen_H
#define GledCore_ZQueen_H

#include <Glasses/ZNameMap.h>
#include <Glasses/SaturnInfo.h>

#include <Gled/MIR_Priest.h>
#include <Gled/GTime.h>

class ZKing; class ZQueen;
class ZComet;
class Ray;

typedef set<ZQueen*>		spZQueen_t;
typedef set<ZQueen*>::iterator	spZQueen_i;

class ZQueen : public ZNameMap, public An_ID_Demangler, public MIR_Priest
{

  // 7777 RnrCtrl(0)
  MAC_RNR_FRIENDS(ZQueen);

  friend class ZKing;
  friend class Saturn;

public:
  //enum Contents_e { QC_Static = 0, QT_Dynamic = 1 };

  enum AuthMode_e { AM_None=0, AM_Queen, AM_Lens,
		    AM_QueenThenLens, AM_LensThenQueen };
  enum Align_e    { A_Good=0, A_Evil };

  enum QueenState_e { QS_Sleeping, QS_Awakening,
		      QS_Dreaming, QS_Nightmare,
		      QS_AwaitingSceptre, QS_Ruling,
		      QS_RelinquishingSceptre };

  enum ZeroRCPolicy_e { ZRCP_Delete, ZRCP_ToOrphanage, ZRCP_Ignore };

  class LensDetails {
  public:
    enum        LensState_e    { LS_Undef, LS_Alive, LS_Purged, LS_Dead };
    ZGlass*	mLens;
    LensState_e mState;
    GTime	mCreationTime;
    GTime	mDeletionTime;

    LensDetails(ZGlass* l, LensState_e ls=LS_Alive) :
      mLens(l), mState(ls), mCreationTime(GTime::I_Now) {}
  };

private:
  void _init();

protected:

  ZKing*	mKing;		  //! X{g}

  Bool_t	bMandatory;	  //  X{gS} 7 Bool(-join=>1)
  Bool_t	bFollowDeps;	  //  X{gS} 7 Bool() Used by fire-space queens

  Bool_t	bRuling;	  //! X{gS} 7 BoolOut(-join=>1)
  Bool_t	bAwaitingSceptre; //! X{gS} 7 BoolOut()
  QueenState_e	mState;	          //! X{gS} 7 PhonyEnum(-const=>1, -width=>12)

  ID_t		mMinID;		// X{g} 7 ValOut(-range=>[0,MAX_ID], -width=>8, -join=>1)
  ID_t		mMaxID;		// X{g} 7 ValOut(-range=>[0,MAX_ID], -width=>8)
  ID_t		mIDSpan;	// X{g} 7 ValOut(-join=>1)
  ID_t		mIDsUsed;	// X{g} 7 ValOut()
  ID_t		mIDsPurged;	// X{g} 7 ValOut(-join=>1)
  ID_t		mIDsFree;	// X{g} 7 ValOut()

  typedef map<ID_t, LensDetails*>		QueenIDMap_t;
  typedef map<ID_t, LensDetails*>::iterator	QueenIDMap_i;

  QueenIDMap_t  mIDMap;		//!

  ID_t		mCreationID;	//  Creation point ID
  lID_t		mPurgatory;	//  List of released but still reserved ids

  UShort_t	mAvgPurgLen;	// X{gS} 7 Value(-range=>[1,65000,1], -width=>5, -join=>1)
  Float_t	mSgmPurgLen;	// X{gS} 7 Value(-range=>[0,1,1,100], -width=>5)
  UInt_t	mPurgedMS;	// X{gS} 7 Value(-range=>[100,1e6,10], -width=>7, -join=>1)
  UInt_t	mDeletedMS;	// X{gS} 7 Value(-range=>[100,1e6,10], -width=>7)

  ZeroRCPolicy_e mZeroRCPolicy; // X{gS} 7 PhonyEnum()

  Bool_t	bStamping;	// X{gS} 7 Bool()

  UChar_t	mAuthMode;      // X{gS} 7 PhonyEnum(-type=>AuthMode_e, -width=>12)
  UChar_t	mAlignment;     // X{gS} 7 PhonyEnum(-type=>Align_e)
  UChar_t	mMapNoneTo;     // X{gS} 7 PhonyEnum(-type=>ZMirFilter::Result_e,
                                //                   -names=>[R_Allow,R_Deny])

  ZMirFilter*	mProtector;     // X{gS} L{}

  ZHashList*	mDeps;          // X{gS} L{}
  ZHashList*	mOrphans;       // X{gS} L{}

  lpSaturnInfo_t mReflectors;	//!

  // Mutexen
  GMutex mSubjectWriteMutex;    //!
  GMutex mSubjectRefCntMutex;   //!


  virtual void bootstrap();
  virtual void embrace_of_life(ZComet& comet);
 
  bool                 has_free_ids(ID_t n_needed);
  ID_t                 next_free_id(QueenIDMap_i i);
  ID_t                 assign_id(ZGlass* lens);
  virtual LensDetails* produce_lens_details(ID_t id, ZGlass* lens);
  void                 release_purgatory(ID_t n_needed);
  void                 release_moon_purgatory(ID_t n_to_release); // X{E}

  virtual ZGlass* instantiate(FID_t fid,
		    const Text_t* name=0, const Text_t* title=0); // X{E}

  // reflector handling; called by Saturn
  void add_reflector(SaturnInfo* moon);
  void remove_reflector(SaturnInfo* moon);


public:

  ZQueen(const Text_t* n="ZQueen", const Text_t* t=0) :
    ZNameMap(n,t), mIDSpan(0),
    mSubjectWriteMutex(GMutex::recursive),
    mSubjectRefCntMutex(GMutex::recursive)
  { _init(); }

  ZQueen(ID_t span, const Text_t* n="ZQueen", const Text_t* t=0) :
    ZNameMap(n,t), mIDSpan(span),
    mSubjectWriteMutex(GMutex::recursive),
    mSubjectRefCntMutex(GMutex::recursive)
  { _init(); }

  // Subject Write/RefCnt locks.
  void SubjectWriteLock()    { mSubjectWriteMutex.Lock(); }
  void SubjectWriteUnlock()  { mSubjectWriteMutex.Unlock(); }
  void SubjectRefCntLock()   { mSubjectRefCntMutex.Lock(); }
  void SubjectRefCntUnlock() { mSubjectRefCntMutex.Unlock(); }

  // ID & Lens management
  virtual ZGlass* DemangleID(ID_t id);
  ID_t CheckIn(ZGlass* lens);


  // MIR blessing
  virtual void BlessMIR(ZMIR& mir);
  virtual void DepCheckMIR(ZMIR& mir);

  // Instantiation methods
  ID_t InstantiateWAttach(ZGlass* attach_to, ZGlass* attach_gamma,
			  LID_t att_lid, CID_t att_cid, MID_t att_mid,
			  LID_t new_lid, CID_t new_cid,
			  const Text_t* name=0, const Text_t* title=0
			  ); // X{E} C{2} T{ZQueen::Instantiate}
  ID_t IncarnateWAttach(ZGlass* attach_to, ZGlass* attach_gamma,
			LID_t att_lid, CID_t att_cid, MID_t att_mid
			); // X{E} C{2} T{ZQueen::Instantiate}

  // instantiate URL

  // Lens deletion
  void PutLensToPurgatory(ZGlass* lens); // X{E} C{1}
  void PutLensToVoid(ID_t lens_id);      // X{E}
  void RemoveLens(ZGlass* lens);         // X{E} C{1}

  // ZeroRefCount and management of Orphans
  void ZeroRefCount(ZGlass* lens);
  void CleanOrphanage();           // X{E} 7 MButt()

  // Mandatory queens are pushed to all lower Saturns
  // being processed ...
  // void SetMandatory(Bool_t mandp);

  // Dependencies
  void AddDependency(ZQueen* new_dep);  // X{E} C{1}
  //void RemoveDependency(ZQueen* dep); // X{E} C{1}
  Bool_t DependsOn(ZQueen* some_queen);

  // Reflections ... operated by Kings
  virtual void CreateReflection(TBuffer& buf);
  virtual void InvokeReflection(TBuffer& buf);
  virtual void RevokeReflection();

  // Comet operations
  ZComet* MakeComet();
  void AdoptComet(ZList* top_dest, ZList* orphan_dest, ZComet* comet); // X{E} C{2}
  virtual void UnfoldFrom(ZComet& comet);

  // Maintenance of non-ruling queen reflections
  void BroadcastMIRToNonRulingReflections(ZMIR& mir);
  void BasicQueenChange(ZMIR& mir);

  // lens stamping
  void EmitRay(auto_ptr<Ray>& ray);

  // tmp
  void ListAll();		 //! X{E} 7 MButt()

#include "ZQueen.h7"
  ClassDef(ZQueen, 1)
}; // endclass ZQueen

GlassIODef(ZQueen);

// Attempt at server exec + broadcast
#ifndef __CINT__
#define CALL_AND_BROADCAST(_lens_, _method_, ...) \
  _lens_->_method_(__VA_ARGS__); \
  { auto_ptr<ZMIR> mir(_lens_->S_##_method_(__VA_ARGS__)); \
    mSaturn->markup_posted_mir(*mir); \
    mSaturn->BroadcastMIR(*mir, mReflectors); }
#endif

#endif
