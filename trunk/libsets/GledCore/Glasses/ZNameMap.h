// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZNameMap_H
#define GledCore_ZNameMap_H

#include <Glasses/ZList.h>

class ZNameMap : public ZList, public ZGlass::NameChangeCB
{
  MAC_RNR_FRIENDS(ZNameMap);

private:
  void _init();

protected:
  // AList ... ZList
  virtual void new_element_check(ZGlass* lens);
  virtual void clear_list();

  // ZList
  virtual void on_insert(ZList::iterator it);
  virtual void on_remove(ZList::iterator it);
  virtual void on_rebuild();

  //----------------------------------------------------------------------

  Bool_t      bKeepSorted;      // X{GE} 7 Bool()
  Bool_t      bReplaceOnInsert; // X{GS} 7 Bool(-join=>1)
  Bool_t      bReplaceOnRename; // X{GS} 7 Bool()

#ifndef __CINT__
  typedef map<TString, ZList::iterator>           mName2Iter_t;
  typedef map<TString, ZList::iterator>::iterator mName2Iter_i;

  mName2Iter_t mItMap; //!
#endif

public:
  ZNameMap(const Text_t* n="ZNameMap", const Text_t* t=0) : ZList(n,t)
  { _init(); }

  void SetKeepSorted(Bool_t keep_sorted);

  virtual void    Add(ZGlass* lens); // Exported in AList

  virtual ZGlass* GetElementByName(const TString& name);

  virtual void SortByName();

  //----------------------------------------------------------------------

  // ZGlass::NameChangeCB
  virtual void name_change_cb(ZGlass* g, const TString& new_name);

#include "ZNameMap.h7"
  ClassDef(ZNameMap, 1)
}; // endclass ZNameMap


#endif
