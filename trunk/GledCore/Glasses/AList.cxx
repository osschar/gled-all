// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AList
//
//

#include "AList.h"
#include "AList.c7"
#include "ZQueen.h"

ClassImp(AList);

/**************************************************************************/

void AList::_init()
{
  mSize = 0;
  mLid  = 0; mCid  = 0;
}

/**************************************************************************/
// ZGlass reference management, extensions for lists, protected part.
/**************************************************************************/

void AList::reference_all()
{
  reference_links();
  reference_list_elms();
}

void AList::unreference_all()
{
  unreference_links();
  unreference_list_elms();
}

void AList::reference_list_elms()
{
  GMutexHolder lck(mListMutex);
  Stepper<> s(this);
  while(s.step())
    s->IncRefCount(this);
}

void AList::unreference_list_elms()
{
  GMutexHolder lck(mListMutex);
  Stepper<> s(this);
  while(s.step())
    s->DecRefCount(this);
}

/**************************************************************************/
// AList methods, protected part.
/**************************************************************************/

void AList::new_element_check(ZGlass* lens)
{
  static const Exc_t _eh("AList::new_element_check ");

  if(lens == 0 && elrep_can_hold_zero() == false) {
    throw(_eh + "called with null ZGlass*.");
  }
  if(mLid && mCid) {
    if(!GledNS::IsA(lens, FID_t(mLid, mCid))) {
      throw(_eh + "lens of wrong FID_t.");
    }
  }
}

/**************************************************************************/
// ZGlass reference management, extensions for lists, public part.
/**************************************************************************/

Int_t AList::RebuildAllRefs(An_ID_Demangler* idd)
{
  return RebuildLinkRefs(idd) + RebuildListRefs(idd);
}

/**************************************************************************/

void AList::ClearAllReferences()
{
  PARENT_GLASS::ClearAllReferences();
  ClearList();
}

/**************************************************************************/

void AList::RemoveLensesViaQueen(Bool_t recurse)
{
  // Sends MIR to queen and waits for result.
  // This should be called from a detached thread.

  if(IsEmpty()) return;

  auto_ptr<ZMIR>    mir( mQueen->S_RemoveLenses(this, recurse) );
  auto_ptr<ZMIR_RR> res( mSaturn->ShootMIRWaitResult(mir) );
  if(res->HasException())
    throw(Exc_t(res->Exception.Data()));
}

/**************************************************************************/
// AList methods, public part.
/**************************************************************************/

void AList::SetElementFID(FID_t fid)
{
  mLid = fid.lid; mCid = fid.cid;
  Stamp(FID());
}

/**************************************************************************/

Bool_t AList::Has(ZGlass* g)
{
  GMutexHolder lck(mListMutex);
  Stepper<> s(this);
  while(s.step())
    if(*s == g) return true;
  return false;
}

/**************************************************************************/

TimeStamp_t AList::CopyList(lpZGlass_t& dest, bool copy_zeros)
{
  GMutexHolder lck(mListMutex);
  Stepper<> s(this, copy_zeros);
  while(s.step())
    dest.push_back(*s);
  return mListTimeStamp;
}

TimeStamp_t AList::CopyListElReps(lElRep_t& dest, bool copy_zeros)
{
  GMutexHolder lck(mListMutex);
  Stepper<> s(this, copy_zeros);
  while(s.step())
    dest.push_back(s.get_elrep());
  return mListTimeStamp;
}

/**************************************************************************/

ZGlass* AList::GetElementByName(const TString& name)
{
  GMutexHolder lck(mListMutex);
  Stepper<> s(this);
  while(s.step()) {
    if(name == s->RefName()) return *s;
  }
  return 0;
}

/**************************************************************************/

void AList::DumpElements(Bool_t dump_zeros)
{
  static const Exc_t _eh("AList::DumpElements ");

  GMutexHolder lck(mListMutex);
  printf("%s %s, mSize=%d\n", _eh.Data(), Identify().Data(), mSize);
  Stepper<> s(this, dump_zeros);
  while(s.step()) {
    ElRep elr = s.get_elrep();
    printf("%4d %10p <%s> %s\n", elr.fId, elr.fLens, elr.fLabel.Data(),
	   elr.fLens ? elr.fLens->Identify().Data() : "<none>");
  }
}


/**************************************************************************/
// Make MIR functions.
/**************************************************************************/

ZMIR* AList::MkMir_Add(ZGlass* lens)
{
  return S_Add(lens);
}

ZMIR* AList::MkMir_RemoveAll(ZGlass* lens)
{
  return S_RemoveAll(lens);
}

/**************************************************************************/

ZMIR* AList::MkMir_PushBack(ZGlass* lens)
{
  TString mname;
  if(list_deque_ops())
    mname = "PushBack";
  else
    mname = "Add";
  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  return mi->MakeMir(this, lens);
}

ZMIR* AList::MkMir_PopBack()
{
  static const Exc_t _eh("AList::MkMir_PopBack ");
  static const TString mname = "PopBack";

  if(!list_deque_ops())
    throw(_eh + mname + " not supported.");
  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  return mi->MakeMir(this);
}

ZMIR* AList::MkMir_PushFront(ZGlass* lens)
{
  TString mname;
  if(list_deque_ops())
    mname = "PushFront";
  else
    mname = "Add";
  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  return mi->MakeMir(this, lens);
}

ZMIR* AList::MkMir_PopFront()
{
  static const Exc_t _eh("AList::MkMir_PopFront ");
  static const TString mname = "PopFront";

  if(!list_deque_ops())
    throw(_eh + mname + " not supported.");

  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  return mi->MakeMir(this);
}

/**************************************************************************/

ZMIR* AList::MkMir_Insert(ZGlass* lens, ElRep& elrep)
{
  static const Exc_t _eh("AList::MkMir_Insert ");

  TString mname("Insert");
  ElType_e et = el_type();
  switch(et) {
  case ET_Lens:
    if(!list_insert_lens_ops())
      throw(_eh + mname + " 'by-lens' not supported.");
    break;
  case ET_Id:
    if(!list_insert_id_ops())
      throw(_eh + mname + " 'by-id' not supported.");
    mname += "ById";
    break;
  default:
    throw(_eh + mname + " not available for this container type.");
  }

  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  ZMIR* mir =  mi->MakeMir(this, lens, et == ET_Lens ? elrep.get_lens() : 0);
  if(et == ET_Id)
    *mir << elrep.get_id();
  return mir;
}

ZMIR* AList::MkMir_Remove(ElRep& elrep)
{
  static const Exc_t _eh("AList::MkMir_Remove ");

  TString mname("Remove");
  ElType_e et = el_type();
  switch(et) {
  case ET_Lens:
    if(!list_insert_lens_ops())
      throw(_eh + mname + "'by-lens' not supported.");
    break;
  case ET_Id:
    if(!list_insert_id_ops())
      throw(_eh + mname + " 'by-id' not supported.");
    mname += "ById";
    break;
  case ET_Label:
    if(!list_set_label_ops())
      throw(_eh + mname + " 'by-label' not supported.");
    mname += "Label";
    break;
  default:
    throw(_eh + mname + " not available for this container type.");
  }

  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  ZMIR* mir =  mi->MakeMir(this, et == ET_Lens ? elrep.get_lens() : 0);
  if(et == ET_Id)
    *mir << elrep.get_id();
  else if(et == ET_Label)
    *mir << elrep.ref_label();
  return mir;
}

/**************************************************************************/

ZMIR* AList::MkMir_SetElement(ZGlass* lens, ElRep& elrep)
{
  static const Exc_t _eh("AList::MkMir_SetElement ");

  TString mname("SetElement");
  ElType_e et = el_type();
  switch(et) {
  case ET_Id:
    if(!list_set_id_ops())
      throw(_eh + mname + " 'by-id' not supported.");
    mname += "ById";
    break;
  case ET_Label:
    if(!list_set_label_ops())
      throw(_eh + mname + " 'by-label' not supported.");
    mname += "ByLabel";
    break;
  default:
    throw(_eh + mname + " not available for this container type.");
  }

  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  ZMIR* mir =  mi->MakeMir(this, lens);
  if(et == ET_Id)
    *mir << elrep.get_id();
  else if(et == ET_Label)
    *mir << elrep.get_label();
  return mir;
}

/**************************************************************************/

ZMIR* AList::MkMir_AddLabel(const TString& label)
{
  static const Exc_t _eh("AList::MkMir_AddLabel ");
  static const TString mname("AddLabel");

  if(!list_set_label_ops())
    throw(_eh + mname + " not supported.");

  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  ZMIR* mir = mi->MakeMir(this);
  *mir << label;
  return mir;
}

ZMIR* AList::MkMir_RemoveLabel(const TString& label)
{
  static const Exc_t _eh("AList::MkMir_RemoveLabel ");
  static const TString mname("RemoveLabel");

  if(!list_set_label_ops())
    throw(_eh + mname + " not supported.");

  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  ZMIR* mir = mi->MakeMir(this);
  *mir << label;
  return mir;
}

ZMIR* AList::MkMir_ChangeLabel(const TString& label, TString new_label)
{
  static const Exc_t _eh("AList::MkMir_ChangeLabel ");
  static const TString mname("ChangeLabel");

  if(!list_set_label_ops())
    throw(_eh + mname + " not supported.");

  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  ZMIR* mir = mi->MakeMir(this);
  *mir << label << new_label;
  return mir;
}

ZMIR* AList::MkMir_InsertLabel(const TString& label, const TString& before)
{
  static const Exc_t _eh("AList::MkMir_InsertLabel ");
  static const TString mname("InsertLabel");

  if(!list_insert_label_ops())
    throw(_eh + mname + " not supported.");

  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  ZMIR* mir = mi->MakeMir(this);
  *mir << label << before;
  return mir;
}

ZMIR* AList::MkMir_InsertByLabel(ZGlass* lens, const TString& label, const TString& before)
{
  static const Exc_t _eh("AList::MkMir_InsertByLabel ");
  static const TString mname("InsertByLabel");

  if(!list_insert_label_ops())
    throw(_eh + mname + " not supported.");

  GledNS::MethodInfo* mi = VGlassInfo()->FindMethodInfo(mname, true);
  ZMIR* mir = mi->MakeMir(this, lens);
  *mir << label << before;
  return mir;
}


/**************************************************************************/
// Stamping.
/**************************************************************************/

// !! The analogous calls in ZGlass have lid/cid counterparts.
// Didn't need them for lists so far. And don't see why I would.
// Just slightly non-consistent.

TimeStamp_t AList::StampListPushBack(ZGlass* lens, Int_t id)
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_push_back, stamp,
		Ray::EB_StructuralChange)
  {
    ray->SetBeta(lens); ray->SetBeta(id);
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t AList::StampListPopBack()
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_pop_back, stamp,
		Ray::EB_StructuralChange)
  {
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t AList::StampListPushFront(ZGlass* lens, Int_t id)
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_push_front, stamp,
		Ray::EB_StructuralChange)
  {
    ray->SetBeta(lens); ray->SetBeta(id);
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t AList::StampListPopFront()
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_pop_front, stamp,
		Ray::EB_StructuralChange)
  {
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

/**************************************************************************/

TimeStamp_t AList::StampListInsert(ZGlass* lens, Int_t id, ZGlass* before)
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_insert, stamp,
		Ray::EB_StructuralChange)
  {
    ray->SetBeta(lens); ray->SetBeta(id);
    ray->SetGamma(before);
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t AList::StampListInsert(ZGlass* lens, Int_t id, Int_t before_id)
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_insert, stamp,
		Ray::EB_StructuralChange)
  {
    ray->SetBeta(lens); ray->SetBeta(id);
    ray->SetGamma(before_id);
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t AList::StampListRemove(ZGlass* lens)
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_remove, stamp,
		Ray::EB_StructuralChange)
  {
    ray->SetBeta(lens);
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t AList::StampListRemove(ZGlass* lens, Int_t id)
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_remove, stamp,
		Ray::EB_StructuralChange)
  {
    ray->SetBeta(lens);
    ray->SetBeta(id);
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t AList::StampListElementSet(ZGlass* lens, Int_t id)
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_element_set, stamp,
		Ray::EB_StructuralChange)
  {
    ray->SetBeta(lens);
    ray->SetBeta(id);
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t AList::StampListElementSet(ZGlass* lens, const TString& label)
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_element_set, stamp,
		Ray::EB_StructuralChange)
  {
    ray->SetBeta(lens);
    ray->SetBeta(label);
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t AList::StampListInsertLabel(ZGlass* lens, const TString& label, const TString& before)
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_insert_label, stamp,
		Ray::EB_StructuralChange)
  {
    ray->SetBeta(lens);
    ray->SetBeta(label);
    ray->SetGamma(before);
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t AList::StampListRemoveLabel(ZGlass* lens, const TString& label)
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_remove_label, stamp,
		Ray::EB_StructuralChange)
  {
    ray->SetBeta(lens);
    ray->SetBeta(label);
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

/**************************************************************************/

TimeStamp_t AList::StampListRebuild()
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_rebuild, stamp,
		Ray::EB_StructuralChange)
  {
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

TimeStamp_t AList::StampListClear()
{
  TimeStamp_t stamp = mListTimeStamp = ++mTimeStamp;
  IF_ZGLASS_RAY(this, RayNS::RQN_list_clear, stamp,
		Ray::EB_StructuralChange)
  {
    ZGLASS_SEND_RAY;
  }
  return stamp;
}

/**************************************************************************/
/**************************************************************************/

// Those two in FTW_Branch. Could move them here if needed.
// AList::ElRep AList::BetaElRep(Ray& ray)  {}
// AList::ElRep AList::GammaElRep(Ray& ray) {}

/**************************************************************************/
// ROOT Streamer.
/**************************************************************************/

void AList::Streamer(TBuffer &b)
{
  static const Exc_t _eh("AList::Streamer ");
  UInt_t R__s, R__c;

  if(b.IsReading()) {

    Version_t R__v = b.ReadVersion(&R__s, &R__c); if(R__v) { }
    ZGlass::Streamer(b);
    b >> mLid >> mCid >> mSize;
    ISdebug(D_STREAM, GForm("%sreading %d elements (%d,%d).",
			    _eh.Data(), mSize, mLid, mCid));
   b.CheckByteCount(R__s, R__c, AList::IsA());

  } else {

    R__c = b.WriteVersion(AList::IsA(), kTRUE);
    ZGlass::Streamer(b);
    b << mLid << mCid << mSize;
    ISdebug(D_STREAM, GForm("%swriting %d elements (%d,%d).",
			    _eh.Data(), mSize, mLid, mCid));
    b.SetByteCount(R__c, kTRUE);

  }
}

/**************************************************************************/
