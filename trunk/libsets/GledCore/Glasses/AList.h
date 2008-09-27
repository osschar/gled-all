// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_AList_H
#define GledCore_AList_H

#include <Glasses/ZGlass.h>

/**************************************************************************/
// AList - abstract base glass for collections of lens references         //
/**************************************************************************/

class AList : public ZGlass
{
  MAC_RNR_FRIENDS(AList);

  friend class Saturn;
  friend class ZQueen;

public:

  /************************************************************************/
  // ElRep - list-element representation; needed for list ops and GUI     //
  /************************************************************************/

  class ElRep
  {
  public:
    ZGlass*  fLens;
    Int_t    fId;
    TString  fLabel;

    ElRep(ZGlass* l=0) : fLens(l), fId(-1) {}
    ElRep(ZGlass* l, Int_t i) : fLens(l), fId(i) {}
    ElRep(ZGlass* l, const TString& lab) : fLens(l), fId(-1), fLabel(lab) {}
    ElRep(ZGlass* l, Int_t i, const TString& lab) : fLens(l), fId(i), fLabel(lab) {}

    ZGlass* get_lens()  const { return fLens;  }
    Int_t   get_id()    const { return fId;    }
    TString get_label() const { return fLabel; }
    const TString& ref_label() const { return fLabel; }
  };

  typedef list<ElRep>		lElRep_t;
  typedef list<ElRep>::iterator	lElRep_i;

  // Three types of *unique* element identification.
  // Each sub-class must support *exactly* one method.
  // 13.8.06: why is ET_Nil not ok? Setting it for ZDeque (was ET_Lens so far).
  enum ElType_e { ET_Nil, ET_Lens, ET_Id, ET_Label };

  /************************************************************************/
  // Stepper, generick iteration-driver. Makes rootcint nervous.
  /************************************************************************/

#ifndef __CINT__

  class stepper_base {
  protected:
    bool first_p;
    bool return_zeros;
  public:
    stepper_base(bool rz=false) : first_p(true), return_zeros(rz) {}
    virtual ~stepper_base() {}
    virtual bool    step()  = 0;
    virtual ZGlass* lens()  = 0;
    virtual ElRep   elrep() = 0;
  };

  template<class Cont>
  class stepper_imp : public stepper_base
  {
  protected:
    typedef typename Cont::iterator iterator_type;

    iterator_type iter, end;

  public:
    stepper_imp(iterator_type b, iterator_type e, bool return_zeros=false) :
      stepper_base(return_zeros), iter(b), end(e) {}

    virtual bool step() {
      if(iter == end) return false;
      if(first_p)
	first_p = false;
      else
	++iter;
      while(iter != end) {
	if(lens() || return_zeros) return true;
	++iter;
      }
      return false;
    }
    virtual ZGlass* lens()  { return iter.lens();  }
    virtual ElRep   elrep() { return iter.elrep(); }
  };

public:

  template<class G=ZGlass>
  class Stepper
  {
  protected:
    stepper_base* m_imp;
    G*            m_cur;

    bool is_ok()
    {
      m_cur = dynamic_cast<G*>(get_lens());
      return (m_cur != 0);
    }
  public:
    Stepper(AList* l, bool return_zeros=false) :
      m_imp(l->make_stepper_imp(return_zeros)) {}
    Stepper(stepper_base* imp) : m_imp(imp) {}
    ~Stepper() { delete m_imp; }

    void reset(AList* l, bool return_zeros=false) {
      delete m_imp;
      m_imp = l->make_stepper_imp(return_zeros);
    }
    bool step() {
      do { if(! m_imp->step()) return false;
      } while(! is_ok());
      return true;
    }
    ZGlass*      get_lens()  { return m_imp->lens();  }
    AList::ElRep get_elrep() { return m_imp->elrep(); }

    G* operator->() { return m_cur; }
    G* operator*()  { return m_cur; }
  };

  virtual stepper_base* make_stepper_imp(bool return_zeros=false) { return 0; }

#endif

private:
  void _init();

protected:
  LID_t			mLid;		//  X{GS} 7 Value(-width=>4, -join=>1)
  CID_t			mCid;		//  X{GS} 7 Value(-width=>4, -join=>1)
  Int_t			mSize;		//  X{G}  7 ValOut(-width=>5)

  TimeStamp_t		mListTimeStamp; //! X{GS}
  GMutex	 	mListMutex;   	//! X{r}

  // ZGlass reference management, extensions for lists.

  virtual void  reference_all();
  virtual void  unreference_all();
  virtual void  reference_list_elms();   // Override for optimization.
  virtual void  unreference_list_elms(); // Override for optimization.
  virtual Int_t remove_references_to(ZGlass* lens) = 0; // ?? Is this ok ?? It is declared and implemented in ZGlass.

  // AList methods.

  virtual void new_element_check(ZGlass* lens);
  virtual void clear_list() = 0;

public:
  AList(const Text_t* n="AList", const Text_t* t=0) :
    ZGlass(n,t), mListMutex(GMutex::recursive) { _init(); }

  // ZGlass reference management, extensions for lists.
  virtual Int_t RebuildAllRefs(An_ID_Demangler* idd);
  virtual Int_t RebuildListRefs(An_ID_Demangler* idd) = 0;

  virtual void ClearAllReferences();
  virtual void ClearList() = 0 ;        // X{E}

  void RemoveLensesViaQueen(Bool_t recurse=false); // X{Ed}

  // List specific interface.

  void   SetElementFID(FID_t fid);      // X{E}
  Int_t  Size()    { return mSize; }
  Bool_t IsEmpty() { return mSize==0; }

  virtual Bool_t Has(ZGlass* g); // Override for optimization!

  // Eventually implement iterators, or sth.

  virtual TimeStamp_t CopyList(lpZGlass_t& dest, bool copy_zeros=false);
  virtual TimeStamp_t CopyListElReps(lElRep_t& dest, bool copy_zeros=false);

  template <class GLASS>
  TimeStamp_t CopyListByGlass(list<GLASS*>& dest) {
    GMutexHolder lck(mListMutex);
    Stepper<GLASS> s(this);
    while(s.step())
      dest.push_back(*s);
    return mListTimeStamp;
  }

  // Searching of elements by name.
  virtual ZGlass* GetElementByName(const TString& name);

  void DumpElements(Bool_t dump_zeros=false); //! X{E} 7 MCWButt()

  //----------------------------------------------------------------------
  // ElRep properties and supported operations.
  // These statements are a promise of implementation for methods below.
  //----------------------------------------------------------------------

  virtual ElType_e el_type()             { return ET_Nil; }

  virtual bool elrep_has_id()            { return false; }
  virtual bool elrep_has_label()         { return false; }
  virtual bool elrep_can_hold_zero()     { return false; }
  virtual bool elrep_can_edit_label()    { return false; }

  virtual bool list_deque_ops()          { return false; }
  virtual bool list_insert_lens_ops()    { return false; }
  virtual bool list_insert_id_ops()      { return false; }
  virtual bool list_set_id_ops()         { return false; }
  virtual bool list_set_label_ops()      { return false; }
  virtual bool list_insert_label_ops()   { return false; }

  //----------------------------------------------------------------------
  // List element handling operations.
  // Not declared here, as there is no unique interface.
  // MIR-makers and GUI rely on naming conventions!
  //----------------------------------------------------------------------

  // General interface
  virtual void Add      (ZGlass* lens) = 0;   // X{E} C{1}
  virtual void RemoveAll(ZGlass* lens) = 0;   // X{E} C{1}

  // Deque interface
  // `````````````````````````````````````````````````````````````````````
  // Pop methods *must* return the poped element if ResultReq is set.
  // virtual ZGlass* FrontElement();          // X{E} C{0}
  // virtual ZGlass* BackElement();           // X{E} C{0}
  // virtual void    PushBack(ZGlass* lens);  // X{E} C{1}
  // virtual ZGlass* PopBack();               // X{E} C{0}
  // virtual void    PushFront(ZGlass* lens); // X{E} C{1}
  // virtual ZGlass* PopFront();              // X{E} C{0}

  // Insert-by-lens-pointer interface;
  // `````````````````````````````````````````````````````````````````````
  // virtual void Insert(ZGlass* lens, ZGlass* before);           // X{E} C{1}
  // virtual void Remove(ZGlass* lens);                           // X{E} C{1}

  // Insert-by-id interface
  // `````````````````````````````````````````````````````````````````````
  // virtual void InsertById(ZGlass* lens, Int_t before_id);      // X{E} C{1}
  // virtual void RemoveById(Int_t id_to_remove);                 // X{E} C{0}

  // Set-by-id interface
  // `````````````````````````````````````````````````````````````````````
  // virtual void SetElementById(ZGlass* lens, Int_t target_id);  // X{E} C{1}

  // Set-by-label interface
  // `````````````````````````````````````````````````````````````````````
  // virtual void SetElementByLabel(ZGlass* lens, TString label,
  //                                Bool_t create=true);          // X{E} C{1}
  // virtual void AddLabel(TString label);                        // X{E} C{0}
  // virtual void RemoveLabel(TString label);                     // X{E} C{0}
  // virtual void ChangeLabel(TString label, TString new_label);  // X{E} C{0}
  //
  // Insert-by-label [external/user-defined label ordering]
  // `````````````````````````````````````````````````````````````````````
  // virtual void InsertByLabel(ZGlass* lens, TString label,
  //                            TString before);                  // X{E} C{1}
  // virtual void InsertLabel(TString label, TString before);     // X{E} C{0}

  //----------------------------------------------------------------------
  // MIR makers. Eventually virtual.
  //----------------------------------------------------------------------

  ZMIR* MkMir_Add(ZGlass* lens);
  ZMIR* MkMir_RemoveAll(ZGlass* lens);

  ZMIR* MkMir_PushBack(ZGlass* lens);
  ZMIR* MkMir_PopBack();
  ZMIR* MkMir_PushFront(ZGlass* lens);
  ZMIR* MkMir_PopFront();

  ZMIR* MkMir_Insert(ZGlass* lens, ElRep& elrep);
  ZMIR* MkMir_Remove(ElRep& elrep);

  ZMIR* MkMir_SetElement(ZGlass* lens, ElRep& elrep);

  ZMIR* MkMir_AddLabel(const TString& label);
  ZMIR* MkMir_RemoveLabel(const TString& label);
  ZMIR* MkMir_ChangeLabel(const TString& label, TString new_label);

  ZMIR* MkMir_InsertByLabel(ZGlass* lens, const TString& label, const TString& before);
  ZMIR* MkMir_InsertLabel(const TString& label, const TString& before);

  //----------------------------------------------------------------------
  // Stamping methods ... coded here for all sub-classes
  //----------------------------------------------------------------------

  // ZGlass virtuals
  virtual void SetStamps(TimeStamp_t s)
  { ZGlass::SetStamps(s); mListTimeStamp = s; }

  virtual TimeStamp_t StampListPushBack(ZGlass* lens, Int_t id=-1);
  virtual TimeStamp_t StampListPopBack();
  virtual TimeStamp_t StampListPushFront(ZGlass* lens, Int_t id=-1);
  virtual TimeStamp_t StampListPopFront();

  virtual TimeStamp_t StampListInsert(ZGlass* lens, Int_t id, ZGlass* before);
  virtual TimeStamp_t StampListInsert(ZGlass* lens, Int_t id, Int_t before_id);
  virtual TimeStamp_t StampListRemove(ZGlass* lens);
  virtual TimeStamp_t StampListRemove(ZGlass* lens, Int_t id);

  virtual TimeStamp_t StampListElementSet(ZGlass* lens, Int_t id);
  virtual TimeStamp_t StampListElementSet(ZGlass* lens, const TString& label);

  virtual TimeStamp_t StampListInsertLabel(ZGlass* lens, const TString& label, const TString& before);
  virtual TimeStamp_t StampListRemoveLabel(ZGlass* lens, const TString& label);

  virtual TimeStamp_t StampListRebuild();
  virtual TimeStamp_t StampListClear();

  // ElRep BetaElRep(Ray& ray);
  // ElRep GammaElRep(Ray& ray);

#include "AList.h7"
  ClassDef(AList, 1); // Abstract container base-class.
}; // endclass AList



#ifndef __CINT__
/* inline template */ template <> inline bool AList::Stepper<ZGlass>::is_ok() { m_cur = get_lens(); return true; }
#endif

#endif
