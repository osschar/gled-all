// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZStringMap_H
#define GledCore_ZStringMap_H

#include <Glasses/AList.h>

class ZStringMap : public AList
{
  MAC_RNR_FRIENDS(ZStringMap);

public:
  typedef map<TString, ZGlass*>  container;
  typedef pair<TString, ZGlass*> element;

  struct iterator : public container::iterator
  {
    ZGlass* lens()  { return (*this)->second; }
    ElRep   elrep() { return ElRep((*this)->second, (*this)->first); }

    ZGlass* operator()() { return (*this)->second; }

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
  // From AList
  virtual Int_t remove_references_to(ZGlass* lens);
  virtual void  clear_list();

  // ZStringMap
  virtual void on_insert(iterator iter) {}
  virtual void on_remove(iterator iter) {}
  virtual void on_change(iterator iter, ZGlass* old_lens) {}
  virtual void on_change_label(iterator iter, TString old_label) {}
  virtual void on_rebuild() {}

  container   mElements;
  TString     mAddLensPrefix; // X{GS} 7 Textor()
  TString     mAddLensFormat; // X{GS} 7 Textor(-width=>6, -join=>1)
  Int_t       mAddLensCurId;  // X{GS} 7 Value(-width=>4)

public:
  ZStringMap(const Text_t* n="ZStringMap", const Text_t* t=0) :
    AList(n,t) { _init(); }
  virtual ~ZStringMap() {}

  virtual Int_t RebuildListRefs(An_ID_Demangler* idd);
  virtual void  ClearList();

  //----------------------------------------------------------------------
  // ElRep properties and supported operations.
  //----------------------------------------------------------------------

  virtual ElType_e el_type()             { return ET_Label; }

  virtual bool elrep_has_label()         { return true; }
  virtual bool elrep_can_hold_zero()     { return true; }
  virtual bool elrep_can_edit_label()    { return true; }

  virtual bool list_set_label_ops()      { return true; }

  //----------------------------------------------------------------------
  // List element handling operations.
  //----------------------------------------------------------------------

  // General interface
  virtual void Add      (ZGlass* lens);
  virtual void RemoveAll(ZGlass* lens);

  // Set-by-label interface
  virtual void SetElementByLabel(ZGlass* lens, TString label); // X{E} C{1}
  virtual void AddLabel(TString label);                        // X{E} C{0}
  virtual void RemoveLabel(TString label);                     // X{E} C{0}
  virtual void ChangeLabel(TString label, TString new_label);  // X{E} C{0}

  virtual ZGlass* GetElementByLabel(TString label); // X{E} C{0}

#include "ZStringMap.h7"
  ClassDef(ZStringMap, 1); // RB-tree with with string indexing (std::map<TString, ZGlass*>).
}; // endclass ZStringMap


#endif
