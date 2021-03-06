// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZHashList_H
#define GledCore_ZHashList_H

#include <Glasses/ZList.h>

class ZHashList : public ZList
{
  MAC_RNR_FRIENDS(ZHashList);

private:
  void _init();

protected:
  virtual Int_t remove_references_to(ZGlass* lens);

  virtual void new_element_check(ZGlass* lens);
  virtual void clear_list();

  virtual void on_insert(ZList::iterator it);
  virtual void on_remove(ZList::iterator it);
  virtual void on_rebuild();

  bool	bNerdyListOps;	    // X{GS} 7 Bool()
#ifndef __CINT__
  typedef hash_map<ZGlass*, ZList::iterator>		hpLens2Iter_t;
  typedef hash_map<ZGlass*, ZList::iterator>::iterator	hpLens2Iter_i;

  hpLens2Iter_t mItHash; //!
#endif

public:
  ZHashList(const Text_t* n="ZHashList", const Text_t* t=0) : ZList(n,t)
  { _init(); }

  virtual ElType_e el_type()             { return ET_Lens; }
  virtual bool list_insert_lens_ops()    { return true; }

  virtual Bool_t Has(ZGlass* lens);

  virtual Int_t  RemoveAll(ZGlass* lens);

  virtual void   Insert(ZGlass* lens, ZGlass* before); // X{E} C{1}
  virtual void   Remove(ZGlass* lens);                 // X{E} C{1}

  // For those two, we have no alist-mir-generators and special stamps.
  // Could be added ... either or both.
  virtual void   MoveToFront(ZGlass* lens);            // X{E} C{1}
  virtual void   MoveToBack (ZGlass* lens);            // X{E} C{1}

  ZGlass* ElementAfter (ZGlass* lens);                 // X{E} C{1}
  ZGlass* ElementBefore(ZGlass* lens);                 // X{E} C{1}

#include "ZHashList.h7"
  ClassDef(ZHashList, 1);
}; // endclass ZHashList


#endif
