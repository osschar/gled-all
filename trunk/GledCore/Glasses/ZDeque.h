// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZDeque_H
#define GledCore_ZDeque_H

#include <Glasses/AList.h>

class ZDeque : public AList
{
  MAC_RNR_FRIENDS(ZDeque);

public:

  typedef list<ZGlass*> container;

  struct iterator : public container::iterator
  {
    ZGlass* lens()  { return **this; }
    ElRep   elrep() { return ElRep(**this); }

    ZGlass* operator()() { return **this; }

    iterator& operator++() { container::iterator::operator++(); return *this; }
    iterator& operator--() { container::iterator::operator--(); return *this; }
    iterator operator++(int) { iterator x(*this); operator++(); return x; }
    iterator operator--(int) { iterator x(*this); operator--(); return x; }

    iterator() {}
    iterator(container::iterator i) : container::iterator(i) {}
  };

  iterator begin() { return mElements.begin(); }
  iterator end()   { return mElements.end(); }

#ifndef __CINT__
  virtual stepper_base* make_stepper_imp(bool return_zeroes=false);
#endif

private:
  void _init();

protected:
  container       mElements; //!

  virtual Int_t remove_references_to(ZGlass* lens);
  virtual void clear_list();

  virtual void on_insert(iterator iter) {}
  virtual void on_remove(iterator iter) {}
  virtual void on_rebuild() {}

public:
  ZDeque(const Text_t* n="ZDeque", const Text_t* t=0) : AList(n,t) { _init(); }

  virtual Int_t RebuildListRefs(An_ID_Demangler* idd);
  virtual void ClearList();

  //----------------------------------------------------------------------
  // ElRep properties and supported operations
  //----------------------------------------------------------------------

  virtual bool list_deque_ops()       { return true; }

  //----------------------------------------------------------------------
  // List element handling operations
  //----------------------------------------------------------------------

  // General interface
  virtual void Add      (ZGlass* lens);    // Exported in AList
  virtual void RemoveAll(ZGlass* lens);    // -"-

  // Deque interface
  virtual ZGlass* FrontElement();          // X{E} C{0}
  virtual ZGlass* BackElement();           // X{E} C{0}
  virtual void    PushBack(ZGlass* lens);  // X{E} C{1}
  virtual ZGlass* PopBack();               // X{E} C{0}
  virtual void    PushFront(ZGlass* lens); // X{E} C{1}
  virtual ZGlass* PopFront();              // X{E} C{0}

#include "ZDeque.h7"
  ClassDef(ZDeque, 1) // Double-ended queue (supports front/back operations)
}; // endclass ZDeque


#endif
