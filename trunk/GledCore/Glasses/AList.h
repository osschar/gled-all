// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_AList_H
#define GledCore_AList_H

#include <Glasses/ZGlass.h>


/**************************************************************************/
// AList - abstract base glass for collections of lens references         //
/**************************************************************************/

class AList : public ZGlass {
  // 7777 RnrCtrl(0)
  MAC_RNR_FRIENDS(AList);

 public:

  /************************************************************************/
  // ElRep - list-element representation; needed for list ops and GUI     //
  /************************************************************************/

  class ElRep {
  public:
    ZGlass*	fLens;

    ElRep() : fLens(0) {}

    virtual ZGlass* get_lens()  { return fLens; }
    virtual Int_t   get_id()    { return 0; }
    virtual TString get_label() { return 0; }

    ClassDef(AList::ElRep, 1)
  };

  typedef list<ElRep>		lElRep_t;
  typedef list<ElRep>::iterator	lElRep_i;

 private:
  void _init();

 protected:
  Int_t			mSize;
  LID_t			mLid;			// X{GS} 7 Value(-join=>1)
  CID_t			mCid;			// X{GS} 7 Value()

  GMutex	 	mListMutex;	   	//!

  virtual void new_element_check(ZGlass* g);

 public:
  AList(const Text_t* n="AList", const Text_t* t=0) :
    ZGlass(n,t), mListMutex(GMutex::recursive) { _init(); }

  virtual void CopyList(lpZGlass_t& dest)     = 0;
  virtual void CopyListElRefs(lElRep_t& dest) = 0;


  virtual Int_t RebuildAllRefs(An_ID_Demangler* idd);
  virtual Int_t RebuildListRefs(An_ID_Demangler* idd) = 0;

  //----------------------------------------------------------------------
  // List element handling operations.
  // Not declared here, as there is no unique interface.
  // GUI relies on naming conventions!
  //----------------------------------------------------------------------

  // Deque interface
  // Pop methods *must* return the poped element if ResultReq is set.
  // virtual void    PushBack(ZGlass* lens);  // X{E} C{1}
  // virtual ZGlass* PopBack();               // X{E} C{0}
  // virtual void    PushFront(ZGlass* lens); // X{E} C{1}
  // virtual ZGlass* PopFront();              // X{E} C{0}

  // List interface; requires element id.
  // virtual void AddBefore(ZGlass* lens, Int_t before_id); // X{E} C{1}

  // By-lens interface;
  // virtual void Add(ZGlass* lens);       // X{E} C{1}
  // virtual void Remove(ZGlass* lens);    // X{E} C{1}

  // By-id interface; requires element id
  // virtual void SetElementById(ZGlass* new_el, Int_t target_id);  // X{E} C{1}
  // virtual void ClearElementById(Int_t target_id);                // X{E} C{0}

  // By-label interface: requires element label
  // virtual void SetElementByLabel(ZGlass* new_el, TString label); // X{E} C{1}
  // virtual void ClearElementByLabel(TString label);               // X{E} C{0}

  // Change-element-label: requires editable labels
  // virtual void ChangeElementLabel(TString old_label, TString new_label); // X{E} C{0}

  //----------------------------------------------------------------------
  // ElRep properties and supported operations
  //----------------------------------------------------------------------

  virtual bool elrep_has_id()         { return false; }
  virtual bool elrep_has_label()      { return false; }
  virtual bool elrep_can_edit_label() { return false; }

  virtual bool list_deque_ops()       { return false; }
  virtual bool list_list_ops()        { return false; }
  virtual bool list_by_lens_ops()     { return false; }
  virtual bool list_by_id_ops()       { return false; }
  virtual bool list_by_label_ops()    { return false; }

  //----------------------------------------------------------------------
  // Stamping methods ... coded here for all sub-classes
  //----------------------------------------------------------------------

  // just copied from ZList ... think labels and set/clear
  /*
  virtual TimeStamp_t	StampListAdd(ZGlass* g, ZGlass* b4);
  virtual TimeStamp_t	StampListRemove(ZGlass* g);
  virtual TimeStamp_t	StampListRebuild();
  virtual TimeStamp_t	StampListClear();
  */

#include "AList.h7"
  ClassDef(AList, 1)
    }; // endclass AList

GlassIODef(AList);

#endif
