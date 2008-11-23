// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_ZCometBag_H
#define Gled_ZCometBag_H

#include <Glasses/ZHashList.h>
class ZSubTree;

class ZComet;

class ZCometBag : public ZHashList
{
  MAC_RNR_FRIENDS(ZCometBag);

private:
  void _init();

protected:
  // ZList
  virtual void on_insert(ZList::iterator it);

  //----------------------------------------------------------------------

  Bool_t	bSmartZNodes;	// X{GS} 7 Bool()
  Int_t		mDepth;		// X{GS} 7 Value(-range=>[-1,100,1,1])
  Bool_t	bFollowLinks;	// X{GS} 7 Bool(-join=>1)
  Bool_t	bFollowLists;	// X{GS} 7 Bool()

  // I don't know ... need something better.
  // A really good stone supporting root TFile::Open(<anything>) stuff.
  TString	mFile;		// X{GS} 7 Filor()
  TString	mPath;		// X{GS} 7 Textor()
  TString	mKeyName;	// X{GS} 7 Textor()

  ZSubTree* make_sub_tree_rep(ZGlass* g);

public:
  ZCometBag(const Text_t* n="ZCometBag", const Text_t* t=0) : ZHashList(n,t) { _init(); }

  virtual void ImportSubTree(ZGlass* lens); // X{E} C{1} 7 MCWButt()
  virtual void WriteComet();                // X{E}      7 MButt()

  ZComet* MakeComet();

#include "ZCometBag.h7"
  ClassDef(ZCometBag, 1);
}; // endclass ZCometBag


#endif
