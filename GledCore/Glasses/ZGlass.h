// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZGlass_H
#define Gled_ZGlass_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>

#include <TObject.h>
#include <TString.h>

/**************************************************************************/

#define GlassIODef(_gls_) \
inline TBuffer &operator>>(TBuffer& b, _gls_*& g) { \
  ID_t id; b >> id; g = (_gls_*)(id); \
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
class TBuffer;	class TMessage;

typedef void (*zglass_stamp_f)(ZGlass*, void*);

class ZGlass : public TObject {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZGlass);
  friend class Saturn;
  friend class ZQueen;
  friend class ZComet;

private:
  void		_init();

protected:
  Saturn*	mSaturn;	//! X{G}
  ZQueen*	mQueen;		//! X{G}
  ZMIR*		mMir;		//! X{G}

  UShort_t	mGlassBits;	//
  TString	mName;		//  X{GE} 7 Textor()
  TString	mTitle;		//  X{GE} 7 Textor()
  ID_t		mSaturnID;	//  X{G}  7 ValOut(-range=>[0,MAX_ID,1,0],
                                //                 -width=>10)
  ZMirFilter*	mGuard;		//  X{E} L{}

  Bool_t	bMIRActive;	//  X{GS} 7 BoolOut(-join=>1)
  Bool_t	bAcceptRefs;	//  X{GS} 7 BoolOut()

  lpZGlass_t	mReverseRefs;	//! lenses that reference *this*
  UShort_t	mRefCount;	//! X{G}  7 ValOut(-width=>4, -join=>1)
  UShort_t	mMoonRefCount;	//! X{G}  7 ValOut(-width=>4)
  UShort_t	mSunRefCount;	//! X{G}  7 ValOut(-width=>4, -join=>1)
  UShort_t	mFireRefCount;	//! X{G}  7 ValOut(-width=>4)

  void set_link_or_die(ZGlass*& link, ZGlass* new_val, LID_t lid, CID_t cid);

  virtual void reference_all();
  virtual void unreference_all();

  virtual void remove_references_to(ZGlass* lens);

  // Mutexen
  mutable GMutex mReadMutex;    //!
 public:
  void ReadLock()   const { mReadMutex.Lock(); }
  void ReadUnlock() const { mReadMutex.Unlock(); }
  void WriteLock();
  void WriteUnlock();

 protected:

  // Handlers of lens-state change notfications ... called from Saturn/Queen
  virtual void AdEnlightenment() {}	// called by Saturn on Enlight
  virtual void AdEndarkenment() {}	// called by Saturn on Endark
  virtual void AdUnfoldment() {}	// called by Queen after comet unpacking

  // MIR stuff
  enum MirComponents_e { MC_Any=0,
			 MC_IsFlare=1, MC_IsBeam=2,
			 MC_HasResultReq=4 };

  void assert_MIR_presence(const string& header, int what=0);

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
  Short_t DecRefCount(ZGlass* from);
  // here also need ReRef(from, old_queen, new_queen) or sth.

  // Link handling
  void         ClearLinks();
  virtual void ClearAllReferences();

  // Queries
  virtual ZGlass* GetLinkByName(const Text_t* link_name);
  virtual ZGlass* GetLinkByName(const string& link_name);

  virtual ZGlass* FindLensByPath(const Text_t* url);
  virtual ZGlass* FindLensByPath(const string& url);

  // Stamps
  virtual void SetStamps(TimeStamp_t s)
  { mTimeStamp = mStampReqTring = s; }

  virtual TimeStamp_t Stamp(LID_t lid, CID_t cid);
  TimeStamp_t Stamp() { return Stamp(0, 0); }
  virtual TimeStamp_t StampLink(LID_t lid, CID_t cid);
  TimeStamp_t StampLink() { return StampLink(0, 0); }

  virtual void MarkStampReqTring()
  { mStampReqTring = ++mTimeStamp; }

  void SetStamp_CB(zglass_stamp_f foo, void* arg);
  void SetStampLink_CB(zglass_stamp_f foo, void* arg);

  virtual Int_t RebuildAll(An_ID_Demangler* idd);

#include "ZGlass.h7"
  ClassDef(ZGlass, 1)
}; // endclass ZGlass

GlassIODef(ZGlass);

#include <Glasses/ZMirFilter.h>

/**************************************************************************/
// Hairy Inlines
/**************************************************************************/

inline
void ZGlass::set_link_or_die(ZGlass*& link, ZGlass* new_val,
			     LID_t lid, CID_t cid)
{
  if(link == new_val) return;
  if(link) link->DecRefCount(this);
  if(new_val) {
    try { new_val->IncRefCount(this); }
    catch(...) {
      if(link) { link = 0; StampLink(lid, cid); }
      throw;
    }
  }
  link = new_val;
  StampLink(lid, cid);
}

#endif
