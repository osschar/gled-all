// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZList_H
#define Gled_ZList_H

#include <Glasses/ZGlass.h>

class ZList;
typedef void (*zlist_stampadd_f)(ZList*, ZGlass*, ZGlass*, void*);
typedef void (*zlist_stampremove_f)(ZList*, ZGlass*, void*);
typedef void (*zlist_stamprebuild_f)(ZList*, void*);

class ZList : public ZGlass {
  // 7777 RnrCtrl("true, 7, RnrBits(2,4,6,0, 0,0,0,5)")
  // **** Custom Streamer ****
  MAC_RNR_FRIENDS(ZList);
  friend class Saturn;
  friend class ZQueen;
private:
  lID_t		mIDs;	 // Temporary store for Streaming
  void	       _init();
protected:
  UInt_t		mSize;
  lpZGlass_t		mGlasses;

  GMutex	 	mListMutex;	   	//!
  zlist_stampadd_f	mStampListAdd_CB;    	//!
  void*			mStampListAdd_CBarg; 	//!
  zlist_stampremove_f	mStampListRemove_CB;    //!
  void*			mStampListRemove_CBarg; //!
  zlist_stamprebuild_f	mStampListRebuild_CB;    //!
  void*			mStampListRebuild_CBarg; //!

public:
  ZList(const Text_t* n="ZList", const Text_t* t=0) :
    ZGlass(n,t), mListMutex(GMutex::recursive) { _init(); }
  virtual ~ZList() {}
  virtual void UnrefAll();

  void Copy(lpZGlass_t& dest);
  template <class GLASS>
  void CopyByGlass(list<GLASS>& dest) {
    mListMutex.Lock();
    for(lpZGlass_i i=mGlasses.begin(); i!=mGlasses.end(); ++i) {
      GLASS g = dynamic_cast<GLASS>(*i);
      if(g) dest.push_back(g);
    }
    mListMutex.Unlock();
  }

  ZGlass* First();
  ZGlass* Last();
  ZGlass* GetByName(const Text_t* name);

  virtual void Add(ZGlass* g);			     // X{E} C{1}
  virtual void AddBefore(ZGlass* g, ZGlass* before); // X{E} C{2}
  virtual void AddFirst(ZGlass* g);		     // X{E} C{1}
  virtual void Remove(ZGlass* g);		     // X{E} C{1}
  virtual void RemoveLast(ZGlass* g);		     // X{E} C{1}

  Int_t	Size() { return mSize; }
  bool	IsEmpty() { return mSize==0; }
  virtual bool	Has(ZGlass* g);

  TimeStamp_t	StampListAdd(ZGlass* g, ZGlass* b4);
  TimeStamp_t	StampListRemove(ZGlass* g);
  TimeStamp_t	StampListRebuild();

  void SetStampListAdd_CB(zlist_stampadd_f foo, void* arg);
  void SetStampListRemove_CB(zlist_stampremove_f foo, void* arg);
  void SetStampListRebuild_CB(zlist_stamprebuild_f foo, void* arg);

  virtual Int_t RebuildList(ZComet* c);

  // void makalooka(); // X{E} 7 MButt()

#include "ZList.h7"
  ClassDef(ZList, 1)
}; // endclass ZList

GlassIODef(ZList);

#endif
