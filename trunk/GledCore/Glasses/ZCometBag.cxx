// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ZCometBag.h"
#include "ZCometBag.c7"
#include <Glasses/ZSubTree.h>
#include <Glasses/ZQueen.h>
#include <Stones/ZComet.h>

ClassImp(ZCometBag)

void ZCometBag::_init()
{
  bSmartZNodes = true;
  mDepth = 1; bFollowLinks = true; bFollowLists = true;
}

/**************************************************************************/

ZSubTree* ZCometBag::make_sub_tree_rep(ZGlass* g)
{
  ZSubTree* st = new ZSubTree(mDepth, bFollowLinks, bFollowLists,
			      GForm("ST of %s", g->GetName()));
  mQueen->CheckIn(st);
  st->SetRoot(g);
  return st;
}

/**************************************************************************/

void ZCometBag::Add(ZGlass* g)
{
  ZSubTree* st = dynamic_cast<ZSubTree*>(g);
  if(!st) st = make_sub_tree_rep(g);
  ZHashList::Add(st);
}

void ZCometBag::AddBefore(ZGlass* g, ZGlass* before)
{
  ZSubTree* st = dynamic_cast<ZSubTree*>(g);
  if(!st) st = make_sub_tree_rep(g);
  ZHashList::AddBefore(st, before);
}

void ZCometBag::AddFirst(ZGlass* g)
{
  ZSubTree* st = dynamic_cast<ZSubTree*>(g);
  if(!st) st = make_sub_tree_rep(g);
  ZHashList::AddFirst(st);
}

/**************************************************************************/

ZComet* ZCometBag::MakeComet()
{
  ZComet* comet = new ZComet(GetName(), GForm("Comet[CometBag] of %s", GetName()));
  WriteLock();

  // !!!! See ZComet, about correct streaming struct
  if(bSmartZNodes) comet->mIgnoredLinks.insert("ZNode::Parent");

  lpZGlass_t sts; Copy(sts);
  for(lpZGlass_i i=sts.begin(); i!=sts.end(); ++i) {
    ZSubTree* t = dynamic_cast<ZSubTree*>(*i);
    comet->AddTopLevel(t->GetRoot(), t->GetFollowLinks(), t->GetFollowLists(),
		      t->GetDepth());
  }
  WriteUnlock();
  return comet;
}

/**************************************************************************/
