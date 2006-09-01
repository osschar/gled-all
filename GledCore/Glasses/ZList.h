// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZList_H
#define GledCore_ZList_H

#include <Glasses/AList.h>

class ZList : public AList
{
  MAC_RNR_FRIENDS(ZList);

  // Element representation

public:
  struct element
  {
    ZGlass* fLens;
    Int_t   fId;
    element(ZGlass* l=0, Int_t i=0) : fLens(l), fId(i) {}
  };

  typedef list<element> container;

  struct iterator : public container::iterator
  {
    ZGlass* lens()  { return (*this)->fLens; }
    ElRep   elrep() { return ElRep((*this)->fLens, (*this)->fId); }

    ZGlass* operator()() { return (*this)->fLens; }

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

protected:
  Int_t           mNextId;
  container       mElements; //! Problem during TStreamerElement creation (it's getting created despite manual streamer).

private:
  void _init();

protected:
  // From AList
  virtual Int_t remove_references_to(ZGlass* lens);
  virtual void  clear_list();

  // ZList
  virtual void on_insert(iterator iter) {}
  virtual void on_remove(iterator iter) {}
  virtual void on_rebuild() {}

public:
  ZList(const Text_t* n="ZList", const Text_t* t=0) :
    AList(n,t) { _init(); }
  virtual ~ZList() {}

  virtual Int_t RebuildListRefs(An_ID_Demangler* idd);
  virtual void  ClearList();

  //----------------------------------------------------------------------
  // ElRep properties and supported operations.
  //----------------------------------------------------------------------

  virtual ElType_e el_type()             { return ET_Id; }

  virtual bool elrep_has_id()            { return true; }

  virtual bool list_deque_ops()          { return true; }
  virtual bool list_insert_id_ops()      { return true; }

  //----------------------------------------------------------------------
  // List element handling operations.
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

  // Insert-by-id interface
  virtual void InsertById(ZGlass* lens, Int_t before_id);      // X{E} C{1}
  virtual void RemoveById(Int_t id_to_remove);                 // X{E} C{0}

  Int_t FindFirst(ZGlass* lens);

  // Pure ZList.

  virtual void SortByName();               // X{ED} 7 MButt()

  // Interface for direct placement of config entries (start-up only).

  ZList* AssertPath(const Text_t* path,
                    const Text_t* new_el_type="ZList"); // X{Ed}
  void Swallow(ZGlass* entry, Bool_t replace_p=true,
	       const Text_t* path="",
	       const Text_t* new_el_type="ZList");
  void Swallow(const Text_t* path, ZGlass* entry);

#include "ZList.h7"
  ClassDef(ZList, 1); // Standard list.
}; // endclass ZList


#endif
