// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZSubTree_H
#define Gled_ZSubTree_H

#include <Glasses/ZGlass.h>

class ZSubTree : public ZGlass {
  MAC_RNR_FRIENDS(ZSubTree);
private:
  void _init();

protected:
  ZGlass*	mRoot;		// X{gS} L{}

  Int_t		mDepth;		// X{gS} 7 Value(-range=>[-1,100,1,1])
  Bool_t	bFollowLinks;	// X{gS} 7 Bool()
  Bool_t	bFollowLists;	// X{gS} 7 Bool()

public:
  ZSubTree(const Text_t* n="ZSubTree", const Text_t* t=0) : ZGlass(n,t)
  { _init(); }

  ZSubTree(Int_t d, Bool_t flnk, Bool_t flst,
	   const Text_t* n="ZSubTree", const Text_t* t=0) :
    ZGlass(n,t),
    mRoot(0), mDepth(d), bFollowLinks(flnk), bFollowLists(flst)
  {}

#include "ZSubTree.h7"
  ClassDef(ZSubTree, 1)
}; // endclass ZSubTree

GlassIODef(ZSubTree);

#endif
