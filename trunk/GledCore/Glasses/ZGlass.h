// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
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
  ID_t id; b >> id; g = (_gls_*)(UInt_t(id)); \
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

private:
  void		_init();

protected:
  Saturn*	mSaturn;	//! X{G}
  ZQueen*	mQueen;		//! X{G}
  ZMIR*		mMir;		//! X{G}

  UShort_t	mGlassBits;	//
  TString	mName;		//  X{GE} 7 Textor()
  TString	mTitle;		//  X{GS} 7 Textor()
  ID_t		mSaturnID;	//  X{G}  7 ValOut(-range=>[0,MAX_ID,1,0],
                                //                 -width=>10)
  ZMirFilter*	mGuard;		//  X{E} L{}

  Bool_t	bMIRActive;	//  X{GS} 7 BoolOut(-join=>1)
  Short_t	mRefCount;	//! X{G}  7 ValOut(-width=>4)
  Short_t	mMoonRefCount;	//!
  Short_t	mSunRefCount;	//!
  Short_t	mFireRefCount;	//!

  // Locks, TimeStaps and CallBacks
  mutable GMutex	 mExecMutex;	   //! X{r} Used by Saturn::executor, Eye, rnrs
  mutable GMutex	 mRefCountMutex;   //! X{r} Used only for linking to/from obj

  TimeStamp_t	 mTimeStamp;	   //! X{GS} TimeStamp of last change
  TimeStamp_t	 mStampReqTring;   //! X{GS} TimeStamp of last change that requires retriangulation

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
protected:
#endif

  // Plain callbacks
  zglass_stamp_f mStamp_CB;	   //! called if non-null
  void*		 mStamp_CBarg;     //!  and the user data
  zglass_stamp_f mStampLink_CB;	   //! called if non-null
  void*		 mStampLink_CBarg; //!  and the user data

  // En(light|dark)enment ... called from Saturn upon light switch
  virtual void AdEnlightenment() {}	// called by Saturn on Enlight
  virtual void AdEndarkenment() {}	// called by Saturn on Endark

  virtual void AdUnfoldment() {}	// called by Queen after comet unpacking

  enum MirComponents_e { MC_IsBeam=1, MC_HasResultReq };
  void AssertMIRPresence(const string& header, int what=0);

public:
  ZGlass(const Text_t* n="ZGlass", const Text_t* t=0) :
    mName(n), mTitle(t),
    mExecMutex(GMutex::recursive),
    mRefCountMutex(GMutex::recursive)
  { _init(); }

  virtual ~ZGlass();

  void SetName(const Text_t* n);
  void SetNameTitle(const Text_t* n, const Text_t* t); // X{E}
  ZMirFilter* GetGuard() const;
  void SetGuard(ZMirFilter* guard);

  // Saturnalia
  bool IsSunSpace();
  bool IsSunOrFireSpace();

  // RefCount
  Short_t IncRefCount(const ZGlass* from);
  Short_t DecRefCount(const ZGlass* from);

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

  void Test_p7_linkspecs();

#include "ZGlass.h7"
  ClassDef(ZGlass, 1)
 }; // endclass ZGlass

GlassIODef(ZGlass);

#include <Glasses/ZMirFilter.h>

#endif
