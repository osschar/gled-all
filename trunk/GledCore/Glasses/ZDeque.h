// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZDeque_H
#define GledCore_ZDeque_H

#include <Glasses/AList.h>

class ZDeque : public AList {
  // 7777 RnrCtrl(0)
  MAC_RNR_FRIENDS(ZDeque);

private:
  void _init();

protected:
  lID_t			mIDs;	        //! Temporary store for Streaming

  Int_t			mSize;
  lpZGlass_t		mLenses;

public:
  ZDeque(const Text_t* n="ZDeque", const Text_t* t=0) : AList(n,t) { _init(); }

  virtual void CopyList(lpZGlass_t& dest);
  virtual void CopyListElRefs(lElRep_t& dest);

  virtual Int_t RebuildListRefs(An_ID_Demangler* idd);
  
  //----------------------------------------------------------------------
  // List element handling operations.
  //----------------------------------------------------------------------

  // Deque interface
  virtual void    PushBack(ZGlass* lens);  // X{E} C{1}
  virtual ZGlass* PopBack();               // X{E} C{0}
  virtual void    PushFront(ZGlass* lens); // X{E} C{1}
  virtual ZGlass* PopFront();              // X{E} C{0}

  //----------------------------------------------------------------------
  // ElRep properties and supported operations
  //----------------------------------------------------------------------

  virtual bool list_deque_ops()       { return true; }


#include "ZDeque.h7"
  ClassDef(ZDeque, 1)
}; // endclass ZDeque

GlassIODef(ZDeque);

#endif
