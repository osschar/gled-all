// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZCometBag_H
#define Gled_ZCometBag_H

#include <Glasses/ZHashList.h>
class ZSubTree;

class ZComet;

class ZCometBag : public ZHashList {
  // 7777 RnrCtrl(0)
  MAC_RNR_FRIENDS(ZCometBag);

private:
  void _init();

protected:
  Bool_t	bSmartZNodes;	// X{GS} 7 Bool()
  Int_t		mDepth;		// X{GS} 7 Value(-range=>[-1,100,1,1])
  Bool_t	bFollowLinks;	// X{GS} 7 Bool(-join=>1)
  Bool_t	bFollowLists;	// X{GS} 7 Bool()

  // I don't know ... need something better.
  // A really good stone supporting root TFile::Open(<anything>) stuff.
  TString	mFile;		// X{GS} 7 Filor()
  TString	mPath;		// X{GS} 7 Textor()

  ZSubTree* make_sub_tree_rep(ZGlass* g);

public:
  ZCometBag(const Text_t* n="ZCometBag", const Text_t* t=0) : ZHashList(n,t) { _init(); }

  virtual void Add(ZGlass* g);			     // X{E} C{1}
  virtual void AddBefore(ZGlass* g, ZGlass* before); // X{E} C{2}
  virtual void AddFirst(ZGlass* g);		     // X{E} C{1}

  ZComet* MakeComet();

#include "ZCometBag.h7"
  ClassDef(ZCometBag, 1)
}; // endclass ZCometBag

GlassIODef(ZCometBag);

#endif
