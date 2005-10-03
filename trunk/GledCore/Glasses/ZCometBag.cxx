// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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

void ZCometBag::on_insert(ZList::iterator it)
{
  PARENT_GLASS::on_insert(it);
  ZSubTree* st = dynamic_cast<ZSubTree*>(it.lens());
  if(!st) st = make_sub_tree_rep(it.lens());
}

/**************************************************************************/

ZSubTree* ZCometBag::make_sub_tree_rep(ZGlass* lens)
{
  ZSubTree* st = new ZSubTree(mDepth, bFollowLinks, bFollowLists,
			      GForm("ST of %s", lens->GetName()));
  mQueen->CheckIn(st);
  st->SetRoot(lens);
  return st;
}

/**************************************************************************/

void ZCometBag::ImportSubTree(ZGlass* lens)
{
  // !!! NOT cluster safe.
  // Should run on a detached thread on the Sun.

  ZSubTree* st = make_sub_tree_rep(lens);
  Add(st);
}

/**************************************************************************/

ZComet* ZCometBag::MakeComet()
{
  ZComet* comet = new ZComet(GetName(), GForm("Comet[CometBag] of %s", GetName()));
  WriteLock();

  // !!!! See ZComet, about correct streaming struct
  if(bSmartZNodes) comet->mIgnoredLinks.insert("ZNode::Parent");

  Stepper<ZSubTree> s(this);
  while(s.step()) {
    comet->AddTopLevel(s->GetRoot(), s->GetFollowLinks(), s->GetFollowLists(),
		       s->GetDepth());
  }
  WriteUnlock();
  return comet;
}

/**************************************************************************/
