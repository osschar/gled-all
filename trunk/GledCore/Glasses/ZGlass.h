// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlass_H
#define GledCore_ZGlass_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>

/**************************************************************************/

#define GlassIODef(_gls_) \
inline TBuffer &operator>>(TBuffer& b, _gls_*& g) { \
  ID_t id; b >> id; char* ptr = 0; ptr += id; g = (_gls_*)(ptr); \
  ISdebug(D_STREAM, GForm("Read _gls_ with id of %u", id)); return b; } \
inline TBuffer &operator<<(TBuffer& b, const _gls_ *g) { \
  if(g) { b << g->GetSaturnID(); ISdebug(D_STREAM, GForm("Writing id of %s, id=%u", g->GetName(), g->GetSaturnID())); } \
  else  { b << 0u; ISdebug(D_STREAM, "Writing id 0"); } \
  return b; }

// LIST_CALL: calls _method_ on all objects of type _type_ in container _cont_
#define LIST_CALL(_cont_, _type_, _method_) \
  for(lpZGlass_i __iter=_cont_.begin(); __iter!=_cont_.end(); ++__iter) { \
    if(_type_* __t = dynamic_cast<_type_*>(*__iter)) { __t->_method_ } }

/**************************************************************************/

namespace ZGlassBits {
  extern UShort_t kFixedName;
  extern UShort_t kDying;
}

/**************************************************************************/

class Saturn;
class ZMIR;
class ZQueen;
class ZMirFilter;
class ZComet;
class GThread;
class TBuffer;	class TMessage;

namespace GledNS {
  class ClassInfo;
  class LinkMemberInfo;
}

typedef void (*zglass_stamp_f)(ZGlass*, void*);

class ZGlass : public TObject {
  MAC_RNR_FRIENDS(ZGlass);
  friend class Saturn;
  friend class ZQueen;
  friend class ZComet;

public:
  struct LinkRep {
    ZGlass*&                fLinkRef;
    GledNS::LinkMemberInfo* fLinkInfo;

    LinkRep(ZGlass*& r, GledNS::LinkMemberInfo* li) :
      fLinkRef(r), fLinkInfo(li) {}
  };

  typedef list<LinkRep>           lLinkRep_t;
  typedef list<LinkRep>::iterator lLinkRep_i;

private:
  void		_init();

protected:
  Saturn*	mSaturn;	//! X{g}
  ZQueen*	mQueen;		//! X{g}

  UShort_t	mGlassBits;	//
  TString	mName;		//  X{RGE} 7 Textor()
  TString	mTitle;		//  X{RGE} 7 Textor()
  ID_t		mSaturnID;	//  X{G}   7 ValOut(-range=>[0,MAX_ID,1,0],
                                //                 -width=>10)
  ZMirFilter*	mGuard;		//  X{E} L{}

  Bool_t	bMIRActive;	//  X{GS} 7 BoolOut(-join=>1)
  Bool_t	bAcceptRefs;	//  X{GS} 7 BoolOut()

#ifndef __CINT__
  hpZGlass2Int_t mReverseRefs;  //! lenses that reference *this*
  void dec_ref_count(hpZGlass2Int_i& i, UShort_t n);
#endif
  UShort_t	mRefCount;	//! X{G}  7 ValOut(-width=>4, -join=>1)
  UShort_t	mMoonRefCount;	//! X{G}  7 ValOut(-width=>4)
  UShort_t	mSunRefCount;	//! X{G}  7 ValOut(-width=>4, -join=>1)
  UShort_t	mFireRefCount;	//! X{G}  7 ValOut(-width=>4)
  UShort_t	mEyeRefCount;	//!

  void set_link_or_die(ZGlass*& link, ZGlass* new_val, FID_t fid);

  virtual void reference_all();
  virtual void unreference_all();

  virtual Int_t remove_references_to(ZGlass* lens);

  // Mutexen
  mutable GMutex mReadMutex;    //!
public:
  void ReadLock()   const { mReadMutex.Lock(); }
  void ReadUnlock() const { mReadMutex.Unlock(); }
  void WriteLock();
  void WriteUnlock();

protected:
  // Detached MIR threads. Stopped by Saturn on CheckOut.
  list<GThread*> mDetachedMIRThreads; //!

  // Handlers of lens-state change notfications ... called from Saturn/Queen
  virtual void AdEnlightenment() {}	// called by Saturn on Enlight
  virtual void AdEndarkenment() {}	// called by Saturn on Endark
  virtual void AdUnfoldment() {}	// called by Queen after comet unpacking

  // MIR stuff
  enum MirComponents_e { MC_Any=0,
			 MC_IsFlare=1, MC_IsBeam=2,
			 MC_HasResultReq=4 };

  ZMIR* get_MIR();
  ZMIR* assert_MIR_presence(const string& header, int what=0);
  ZMIR* suggest_MIR_presence(const string& header, int what=0);

  // TimeStaps
  TimeStamp_t	 mTimeStamp;	   //! X{GS} TimeStamp of last change
  TimeStamp_t	 mStampReqTring;   //! X{GS} TimeStamp of last change that requires retriangulation

  // Plain callbacks
  zglass_stamp_f mStamp_CB;	   //! called if non-null
  void*		 mStamp_CBarg;     //!  and the user data
  zglass_stamp_f mStampLink_CB;	   //! called if non-null
  void*		 mStampLink_CBarg; //!  and the user data

  // Name change callback
public:
  class YNameChangeCB {
  public:
    virtual void y_name_change_cb(ZGlass* g, const string& new_name) = 0;
  };

#ifndef __CINT__
  set<YNameChangeCB*>*	pSetYNameCBs; //!

  void register_name_change_cb(YNameChangeCB* rec);
  void unregister_name_change_cb(YNameChangeCB* rec);
#endif

public:

  ZGlass(const Text_t* n="ZGlass", const Text_t* t=0) :
    mName(n), mTitle(t), mReadMutex(GMutex::recursive)
  { _init(); }

  virtual ~ZGlass();

  void UpdateGlassView();       //! X{E} 7 MButt(-join=>1)
  void UpdateAllViews();        //! X{E} 7 MButt()

  string Identify() const;

  void SetName(const Text_t* n);
  void SetTitle(const Text_t* t);
  ZMirFilter* GetGuard() const;
  void SetGuard(ZMirFilter* guard);

  // Saturnalia
  bool IsSunSpace();
  bool IsSunOrFireSpace();

  // RefCount
  Short_t IncRefCount(ZGlass* from);
  Short_t DecRefCount(ZGlass* from, UShort_t n=1);
  // here also need ReRef(from, old_queen, new_queen) or sth.
  Short_t IncEyeRefCount();
  Short_t DecEyeRefCount();

  // Link handling
  void         ClearLinks();
  virtual void ClearAllReferences();
  Int_t        RemoveReferencesTo(ZGlass* lens);

  // Queries
  virtual ZGlass* GetLinkByName(const Text_t* link_name);
  virtual ZGlass* GetLinkByName(const string& link_name);

  virtual ZGlass* FindLensByPath(const Text_t* url);
  virtual ZGlass* FindLensByPath(const string& url);

  // Stamps
  virtual void SetStamps(TimeStamp_t s)
  { mTimeStamp = mStampReqTring = s; }

  virtual TimeStamp_t Stamp(FID_t fid, UChar_t eye_bits=0);
  TimeStamp_t Stamp()     { return Stamp(FID_t(0,0)); }
  TimeStamp_t StampVFID() { return Stamp(VFID()); }

  virtual TimeStamp_t StampLink(FID_t fid);
  TimeStamp_t StampLink()     { return StampLink(FID_t(0,0)); }
  TimeStamp_t StampLinkVFID() { return StampLink(VFID()); }

  virtual void MarkStampReqTring()
  { mStampReqTring = ++mTimeStamp; }

  void SetStamp_CB(zglass_stamp_f foo, void* arg);
  void SetStampLink_CB(zglass_stamp_f foo, void* arg);

  virtual Int_t RebuildAllRefs(An_ID_Demangler* idd);

#include "ZGlass.h7"
  ClassDef(ZGlass, 1) // Base class of Gled enabled classes.
}; // endclass ZGlass

GlassIODef(ZGlass);

#include <Glasses/ZMirFilter.h>

#endif
