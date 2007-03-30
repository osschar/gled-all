// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


//______________________________________________________________________
// ZList
//
//

#include "ZList.h"
#include "ZList.c7"

#include <Glasses/ZQueen.h>
#include <Glasses/ZKing.h>
#include <Eye/Ray.h>
#include <Stones/ZComet.h>
#include <Gled/GledNS.h>

#include <algorithm>
#include <iterator>

ClassImp(ZList);

/**************************************************************************/

void ZList::_init()
{
  mNextId = 0;
}

/**************************************************************************/

Int_t ZList::remove_references_to(ZGlass* lens)
{
  Int_t n  = ZGlass::remove_references_to(lens);
  GMutexHolder llck(mListMutex);
  for(iterator i=begin(); i!=end(); ++i) {
    if(i() == lens) {
      iterator j = i; --i;
      Int_t id = j->fId;
      on_remove(j);
      mElements.erase(j);
      StampListRemove(lens, id);
      ++n;
    }
  }
  return n;
}

void ZList::clear_list()
{
  mSize = 0;
  mElements.clear();
  mNextId = 0;
}

/**************************************************************************/
// ZGlass reference management, extensions for lists, public part.
/**************************************************************************/

Int_t ZList::RebuildListRefs(An_ID_Demangler* idd)
{
  Int_t ret = 0;
  container in;        // !!! Probaly could do without copy, see ZStringMap
  mElements.swap(in);
  mSize   = 0;
  mNextId = 0;
  for(iterator i=in.begin(); i!=in.end(); ++i) {
    ZGlass* lens = idd->DemangleID(GledNS::CastLens2ID(i->fLens));
    if(lens) {
      try {
	lens->IncRefCount(this);
	mElements.push_back(element(lens, mNextId++)); ++mSize;
      }
      catch(...) {
	++ret;
      }
    } else {
      ++ret;
    }
  }
  on_rebuild();
  return ret;
}

void ZList::ClearList()
{
  static const Exc_t _eh("ZList::ClearList ");

  if(mSize == 0) return;

  container foo;
  ISdebug(1, _eh + GForm("locking list '%s'.", GetName()));
  mListMutex.Lock();
  foo.swap(mElements);
  clear_list();
  StampListClear();
  mListMutex.Unlock();
  ISdebug(1, _eh + GForm("unlocked list '%s'.", GetName()));
  for(iterator i=foo.begin(); i!=foo.end(); ++i) {
    i()->DecRefCount(this);
  }
  ISdebug(1, _eh + GForm("finished for '%s'.", GetName()));
}

/**************************************************************************/
// AList methods, public part.
/**************************************************************************/

AList::stepper_base* ZList::make_stepper_imp(bool return_zeros)
{
  return new stepper_imp<ZList>(begin(), end(), return_zeros);
}

/**************************************************************************/
// Generick
/**************************************************************************/

void ZList::Add(ZGlass* lens)
{
  PushBack(lens);
}

void ZList::RemoveAll(ZGlass* lens)
{
  Int_t n  = 0;
  mListMutex.Lock();
  for(iterator i=begin(); i!=end(); ++i) {
    if(i() == lens) {
      iterator j = i; --i;
      StampListRemove(j->fLens, j->fId);
      mElements.erase(j); --mSize;
      ++n;
    }
  }
  mListMutex.Unlock();
  if(n) lens->DecRefCount(this, n);
}

/**************************************************************************/
// Deque
/**************************************************************************/

ZGlass* ZList::FrontElement()
{
  ZGlass* l;
  { GMutexHolder llck(mListMutex);
    l = mSize ? mElements.front().fLens : 0;
  }
  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    GledNS::WriteLensID(b, l);
    mSaturn->ShootMIRResult(b);
  }
  return l;
}

ZGlass* ZList::BackElement()
{ 
  ZGlass* l;
  { GMutexHolder _lstlck(mListMutex);
    l = mSize ? mElements.back().fLens : 0;
  }
  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    GledNS::WriteLensID(b, l);
    mSaturn->ShootMIRResult(b);
  }
  return l;
}

void ZList::PushBack(ZGlass* lens)
{
  GMutexHolder llck(mListMutex);
  new_element_check(lens);
  lens->IncRefCount(this);
  mElements.push_back(element(lens, mNextId++)); ++mSize;
  on_insert(--end());
  StampListPushBack(lens, mNextId - 1);
}

ZGlass* ZList::PopBack()
{
  static const Exc_t _eh("ZList::PopBack ");

  if(mSize == 0)
    throw(_eh + "list empty.");

  ZGlass* l;
  { GMutexHolder llck(mListMutex);
    l = mElements.back().fLens;
    on_remove(--end());
    mElements.pop_back(); --mSize;
    l->DecRefCount(this);
    StampListPopBack();
  }
  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    GledNS::WriteLensID(b, l);
    mSaturn->ShootMIRResult(b);
  }
  return l;
}

void ZList::PushFront(ZGlass* lens)
{
  GMutexHolder llck(mListMutex);
  new_element_check(lens);
  lens->IncRefCount(this);
  mElements.push_front(element(lens, mNextId++)); ++mSize;
  on_insert(begin());
  StampListPushFront(lens, mNextId - 1);
}

ZGlass* ZList::PopFront()
{
  static const Exc_t _eh("ZList::PopFront ");

  if(mSize == 0)
    throw(_eh + "list empty.");

  ZGlass* l;
  { GMutexHolder llck(mListMutex);
    l = mElements.front().fLens;
    on_remove(begin());
    mElements.pop_front(); --mSize;
    l->DecRefCount(this);
    StampListPopFront();
  }
  ZMIR* mir = get_MIR();
  if(mir && mir->HasResultReq()) {
    TBufferFile b(TBuffer::kWrite);
    GledNS::WriteLensID(b, l);
    mSaturn->ShootMIRResult(b);
  }
  return l;
}

/**************************************************************************/
// Insert by Id
/**************************************************************************/

namespace {
  struct element_eq_id : public unary_function<ZList::element&, bool> {
    Int_t id;
    element_eq_id(Int_t i) : id(i) {}
    bool operator()(const ZList::element& el) { return el.fId == id; }
  };
}

void ZList::InsertById(ZGlass* lens, Int_t before_id)
{
  static const Exc_t _eh("ZList::InsertById ");

  new_element_check(lens);

  GMutexHolder llck(mListMutex);
  iterator i = find_if(begin(), end(), element_eq_id(before_id));
  if(i == end())
    throw(_eh + "element with given id not found.");
  lens->IncRefCount(this);
  mElements.insert(i, element(lens, mNextId++)); ++mSize;
  on_insert(--i);
  StampListInsert(lens, mNextId-1, before_id);
}

void ZList::RemoveById(Int_t id_to_remove)
{
  static const Exc_t _eh("ZList::RemoveById ");

  if(mSize == 0)
    throw(_eh + "list empty.");

  GMutexHolder llck(mListMutex);
  iterator i = find_if(begin(), end(), element_eq_id(id_to_remove));
  if(i == end())
    throw(_eh + "element with given id not found.");
  ZGlass* l = i->fLens;
  on_remove(i);
  mElements.erase(i); --mSize;
  l->DecRefCount(this);
  StampListRemove(l, id_to_remove);
}

Int_t ZList::FindFirst(ZGlass* lens)
{
  static const Exc_t _eh("ZList::FindFirst ");

  GMutexHolder llck(mListMutex);
  for(iterator i=begin(); i!=end(); ++i) {
    if(i->fLens == lens)
      return i->fId;
  }
  throw(_eh + "lens not found.");
}

/**************************************************************************/
// ZList specific
/**************************************************************************/

void ZList::SortByName()
{
  GMutexHolder llck(mListMutex);
  if(mSize < 2) return;
  multimap<TString, ZGlass*> nmap;
  for(iterator i=begin(); i!=end(); ++i) {
    nmap.insert(pair<TString, ZGlass*>(i()->GetName(), i()));
  }
  mElements.clear();
  mNextId = 0;
  for(multimap<TString, ZGlass*>::iterator i=nmap.begin(); i!=nmap.end(); ++i) {
    mElements.push_back(element(i->second, mNextId++));
  }
  on_rebuild();
  StampListRebuild();
}

/**************************************************************************/
// Configuration helpers.
/**************************************************************************/

ZList* ZList::AssertPath(const Text_t* path, const Text_t* new_el_type)
{
  // Makes sure that 'path' exists. From the missing element onwards
  // creates new sub-lists of type 'new_el_type'.
  // Throws an exception if:
  // a) link dereferencing is found in path;
  // b) would have to create a missing element under some other queen;
  // c) element in path is not a list;
  // d) instantiation fails.

  static const Exc_t _eh("ZList::AssertPath ");

  namespace GNS = GledNS;

  FID_t new_el_fid = GNS::FindClassID(new_el_type);
  if(new_el_fid.is_null())
    throw(_eh + "unknown list element type '" + new_el_type + "'.");


  list<GNS::url_token> ts;
  GNS::tokenize_url(path, ts);

  ZList* l = this;
  for(list<GNS::url_token>::iterator i=ts.begin(); i!=ts.end(); ++i) {
    switch (i->type()) {

    case GNS::url_token::link_sel: {
      throw(_eh + "link found but only list elements expected.");
      break;
    }

    case GNS::url_token::list_sel: {
      ZGlass* e = l->GetElementByName(*i);
      if(e != 0) {
	l = dynamic_cast<ZList*>(e);
	if(l == 0)
	  throw(_eh + "path element is not a list.");
      } else {
	if(l->GetQueen() != mQueen)
	  throw(_eh + "path leading to another queen.");

	if(mQueen->GetKing()->GetLightType() == ZKing::LT_Fire) {
	  ZGlass* g = GledNS::ConstructLens(new_el_fid);
	  if(g == 0) throw(_eh + "direct lens construction failed.");
	  g->SetName(i->Data());
	  mQueen->CheckIn(g); l->Add(g);
	  l = dynamic_cast<ZList*>(g);
	} else {
	  auto_ptr<ZMIR> att_mir( l->S_Add(0) );
	  auto_ptr<ZMIR> inst_mir
	    ( mQueen->S_InstantiateWAttach
	      (new_el_fid.lid, new_el_fid.cid, i->Data()) );
	  inst_mir->ChainMIR(att_mir.get());

	  auto_ptr<ZMIR_RR> res( mSaturn->ShootMIRWaitResult(inst_mir) );
	  if(res->HasException())
	    throw(_eh + "got exception: " + res->Exception.Data());
	  if(res->HasResult()) {
	    ID_t id; *res >> id;
	    l = dynamic_cast<ZList*>(mSaturn->DemangleID(id));
	  } else
	    throw(_eh + "bad MIR result.");
	}
	if(l == 0)
	  throw(_eh + "instantiation of missing element failed.");
      }
      break;
    }

    default:
      throw(_eh + "unknown token type.");
      break;

    } // end switch
  } // end for

  return l;
}

void ZList::Swallow(ZGlass* entry, Bool_t replace_p,
		    const Text_t* path, const Text_t* new_el_type)
{
  // A helper function for adding configuration DB entries.
  // Mostly intended for use from scripts.
  // Should NOT be called on exposed object-spaces!

  ZList *l = AssertPath(path, new_el_type);
  if(entry->GetSaturnID() == 0) mQueen->CheckIn(entry);
  ZGlass* ex_entry = l->GetElementByName(entry->GetName());
  if(ex_entry)
    if(replace_p)
      l->RemoveAll(ex_entry);
    else 
      return;
  l->Add(entry);
}

void ZList::Swallow(const Text_t* path, ZGlass* entry)
{
  Swallow(entry, true, path, "ZList");
}


/**************************************************************************/
/**************************************************************************/
// Streamer
/**************************************************************************/

void ZList::Streamer(TBuffer &b)
{
  UInt_t R__s, R__c;

  if(b.IsReading()) {

    Version_t R__v = b.ReadVersion(&R__s, &R__c); if(R__v) { }
    AList::Streamer(b);
    b >> mNextId;
    Int_t el_id;
    ID_t  id;
    mElements.clear();
    for(Int_t i=0; i<mSize; ++i) {
      b >> id >> el_id; 
      mElements.push_back(element((ZGlass*)id, el_id));
    }
    b.CheckByteCount(R__s, R__c, ZList::IsA());

  } else {

    R__c = b.WriteVersion(ZList::IsA(), kTRUE);
    AList::Streamer(b);
    b << mNextId;
    for(iterator i=begin(); i!=end(); ++i)
      b << i()->GetSaturnID() << i->fId;
    b.SetByteCount(R__c, kTRUE);

  }
}

/**************************************************************************/
