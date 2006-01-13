// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZGlass_H
#define GledCore_ZGlass_H

#include <Gled/GledTypes.h>
#include <Gled/GMutex.h>

/**************************************************************************/
// ZLinkBase and ZLink<>
/**************************************************************************/

class ZLinkBase
{
 protected:
  ZGlass* _lens;
 public:
  ZLinkBase()          : _lens(0) {}
  ZLinkBase(ZGlass* l) : _lens(l) {}
  virtual ~ZLinkBase() {}

  void     set_link(ZGlass* l) { _lens = l; }
  ZGlass** ptr_link()          { return &_lens; }
  ZGlass*& ref_link()          { return  _lens; }
  ZGlass*  get_link()          { return  _lens; }
  Bool_t   is_set()            { return  _lens != 0; }
  Bool_t   empty()             { return  _lens == 0; }

  ID_t get_id() { return (ID_t) _lens; }

  ZLinkBase& operator=(ZGlass* l) { _lens = l; return *this; }

  ClassDef(ZLinkBase, 0)
};

template <class T>
class ZLink : public ZLinkBase
{
 public:
  ZLink()     : ZLinkBase()  {}
  ZLink(T* l) : ZLinkBase(l) {}
  virtual ~ZLink() {}


  T* operator*()  { return (T*)_lens; }
  T* operator->() { return (T*)_lens; }

  T*   get()      { return  (T*)_lens; }
  void set(T* l)  { _lens = l; }

  ZLink<T>& operator=(T* l) { _lens = l; return *this; }
  bool operator==(T* l) { return _lens == l; }
  bool operator!=(T* l) { return _lens != l; }

  ClassDef(ZLink<T>, 0)
};

/**************************************************************************/

namespace ZGlassBits {
  extern UShort_t kFixedName;
  extern UShort_t kDying;
}

/**************************************************************************/

class Saturn;
class ZMIR;
class Ray;
class ZQueen;
class ZMirFilter;
class ZComet;
class GThread;
class TBuffer;	class TMessage;

namespace GledNS {
  class ClassInfo;
  class LinkMemberInfo;
}

/**************************************************************************/
// Virtual bases for rulers etc. Should be somewhere else
/**************************************************************************/

class An_ID_Demangler
{
 public:
  virtual ~An_ID_Demangler() {}
  virtual ZGlass* DemangleID(ID_t) = 0;
  ClassDef(An_ID_Demangler, 0)
};

class MIR_Priest
{
  friend class Saturn;
public:
  virtual ~MIR_Priest () {}
protected:
  virtual void BlessMIR(ZMIR& mir) = 0;
  ClassDef(MIR_Priest, 0)
};

/**************************************************************************/
// ZGlass
/**************************************************************************/

class ZGlass : public TObject
{
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
  ID_t		mSaturnID;	//  X{G}   7 ValOut(-range=>[0,MAX_ID,1], -width=>10)
  ZLink<ZMirFilter> mGuard;     //  X{E} L{}

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
  virtual void reference_links();
  virtual void unreference_links();

  virtual Int_t remove_references_to(ZGlass* lens);

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

  ZMIR* get_MIR();
  ZMIR* assert_MIR_presence(const TString& header, int what=0);
  ZMIR* suggest_MIR_presence(const TString& header, int what=0);

  void  warn_caller(const TString& warning);

  // TimeStaps
  TimeStamp_t	 mTimeStamp;	   //! X{GS} TimeStamp of last change
  TimeStamp_t	 mStampReqTring;   //! X{GS} TimeStamp of last change that requires retriangulation

public:

  ZGlass(const Text_t* n="ZGlass", const Text_t* t=0) :
    mName(n), mTitle(t), mReadMutex(GMutex::recursive)
  { _init(); }

  virtual ~ZGlass();

  virtual Int_t RebuildAllRefs(An_ID_Demangler* idd);

  void UpdateGlassView();   //! X{E} 7 MButt(-join=>1)
  void UpdateAllViews();    //! X{E} 7 MButt()
  void ReTriangulate();     //! X{E} 7 MButt(-join=>1)
  void RebuildRnrScheme();  //! X{E} 7 MButt()
protected:
  Bool_t bUseDispList;      // X{GST} 7 Bool(-join=>1)
  Bool_t bUseNameStack;     // X{GS}  7 Bool()
public:

  TString Identify() const;

  void SetName(const Text_t* n);
  void SetTitle(const Text_t* t);

  ZMirFilter* GetGuard();
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
  virtual ZGlass* GetLinkByName(const TString& link_name);
  virtual ZGlass* FindLensByPath(const TString& url, bool throwp=false);

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

  void SetUseDLRec(Bool_t state); // X{ED} 7 MCWButt()

  //----------------------------------------------------------------------
  // RayAbsorber and NameChangeCB
  //----------------------------------------------------------------------

public:
  class RayAbsorber
  {
  public:
    virtual ~RayAbsorber() {}
    virtual void AbsorbRay(Ray& ray) = 0;
    ClassDef(RayAbsorber, 0)
  };
  void register_ray_absorber  (RayAbsorber* ra);
  void unregister_ray_absorber(RayAbsorber* ra);

  class NameChangeCB
  {
  public:
    virtual ~NameChangeCB() {}
    virtual void name_change_cb(ZGlass* g, const TString& new_name) = 0;
    ClassDef(NameChangeCB, 0)
  };
  void register_name_change_cb  (NameChangeCB* nccb);
  void unregister_name_change_cb(NameChangeCB* nccb);

protected:
  set<RayAbsorber*>*  pspRayAbsorber;  //!
  set<NameChangeCB*>* pspNameChangeCB; //!

public:

#include "ZGlass.h7"
  ClassDef(ZGlass, 1) // Base class of Gled enabled classes.
}; // endclass ZGlass

#include <Glasses/ZMirFilter.h>

/**************************************************************************/
// Strange defines.
/**************************************************************************/

// LIST_CALL: calls _method_ on all objects of type _type_ in container _cont_
#define LIST_CALL(_cont_, _type_, _method_) \
  for(lpZGlass_i __iter=_cont_.begin(); __iter!=_cont_.end(); ++__iter) { \
    if(_type_* __t = dynamic_cast<_type_*>(*__iter)) { __t->_method_ } }

#define IF_ZGLASS_RAY(...) { \
  bool queenray_p = mQueen && mSaturn && mSaturn->AcceptsRays(); \
  bool absorber_p = pspRayAbsorber != 0; \
  if(queenray_p || absorber_p) { \
    auto_ptr<Ray> ray(Ray::PtrCtor(__VA_ARGS__));

#define IF_ZGLASS_CHANGE_RAY(...) { \
  bool queenray_p = mQueen && mSaturn && mQueen->GetStamping() && mSaturn->AcceptsRays(); \
  bool absorber_p = pspRayAbsorber != 0; \
  if(queenray_p || absorber_p) { \
    auto_ptr<Ray> ray(Ray::PtrCtor(__VA_ARGS__));

#define ZGLASS_SEND_RAY \
    if(absorber_p) \
      for(set<RayAbsorber*>::iterator i=pspRayAbsorber->begin(); i!=pspRayAbsorber->end(); ++i) \
	(*i)->AbsorbRay(*ray); \
    if(queenray_p) \
      mQueen->EmitRay(ray); \
  } \
}

#endif
