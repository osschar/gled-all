// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZVector_H
#define GledCore_ZVector_H

#include <Glasses/AList.h>

class ZVector : public AList
{
  MAC_RNR_FRIENDS(ZVector);

private:
  void _init();

public:
  typedef vector<ZGlass*> container;

  struct iterator : public container::iterator
  {
    Int_t idx;

    ZGlass* lens()  { return *((container::iterator) *this); }
    ElRep   elrep() { return ElRep(lens(), idx); }

    ZGlass* operator()() { return lens(); }

    iterator& operator++() { container::iterator::operator++(); ++idx; return *this; }
    iterator& operator--() { container::iterator::operator--(); --idx; return *this; }
    iterator operator++(int) { iterator x(*this); operator++(); return x; }
    iterator operator--(int) { iterator x(*this); operator--(); return x; }

    iterator() : container::iterator(), idx(0) {}
    iterator(container::iterator i, Int_t x) : container::iterator(i), idx(x) {}
  };

  iterator begin() { return iterator(mElements.begin(), 0); }
  iterator end()   { return iterator(mElements.end(), mSize); }

#ifndef __CINT__
  virtual stepper_base* make_stepper_imp(bool return_zeroes=false);
#endif

protected:
  // From AList
  virtual Int_t remove_references_to(ZGlass* lens);
  virtual void clear_list();

  // ZVector
  virtual void on_insert(Int_t idx) {}
  virtual void on_remove(Int_t idx) {}
  virtual void on_change(Int_t idx, ZGlass* old_lens) {}
  virtual void on_rebuild() {}

  Float_t      mGrowFac;  // X{GS} 7 Value(-range=>[1,10,1,100], -join=>1, -width=>6)
  Int_t        mGrowStep; // X{GS} 7 Value(-range=>[0,65536,1], -width=>6)
  Int_t        mReserved; // X{G}  7 ValOut(-join=>1, -width=>6)
  Int_t        mNextIdx;  // X{GE} 7 Value(-range=>[0,1e6,1], -width=>6)
  container    mElements;

public:
  ZVector(const Text_t* n="ZVector", const Text_t* t=0) :
    AList(n,t) { _init(); }

  virtual Int_t RebuildListRefs(An_ID_Demangler* idd);
  virtual void ClearList();

  //----------------------------------------------------------------------
  // ElRep properties and supported operations.
  //----------------------------------------------------------------------

  virtual ElType_e el_type()             { return ET_Id; }

  virtual bool elrep_has_id()            { return true; }
  virtual bool elrep_can_hold_zero()     { return true; }

  virtual bool list_set_id_ops()         { return true; }

  //----------------------------------------------------------------------
  // List element handling operations.
  //----------------------------------------------------------------------

  // General interface
  virtual void Add      (ZGlass* lens);   // X{E} C{1}
  virtual void RemoveAll(ZGlass* lens);   // X{E} C{1}

  // Set-by-id interface
  virtual void SetElementById(ZGlass* lens, Int_t index);  // X{E} C{1}

  virtual ZGlass* GetElementById(Int_t index); // X{E} C{0}

  // ZVector
  virtual void Resize(Int_t size);     // X{E} C{0}
  virtual void Reserve(Int_t reserve); // X{E} C{0}
  virtual void ClipReserve();          // X{E} C{0}

  void SetNextIdx(Int_t nextidx);

  // debug
  void dumpVecStat();

#include "ZVector.h7"
  ClassDef(ZVector, 1)
}; // endclass ZVector


#endif
