// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_FTW_Branch_H
#define Gled_FTW_Branch_H

#include "OptoStructs.h"
#include "FTW_Leaf.h"
#include "FTW.h"

class FTW_Branch : public FTW_Leaf {
private:
  FTW::lLoI_t		mLeoim;
  void build_leoim();
  void wipe_leoim();

protected:
  bool			bLeavesCreated;
  bool			bListExpanded;	// mLeafs shown

  virtual void label_namebox();
  virtual void label_weeds();

public:
  FTW_Branch(FTW_Nest* nest, FTW_Leaf* parent,
	     OptoStructs::ZGlassImg* img,
	     bool is_list_member, bool is_link_desc);
  virtual ~FTW_Branch();

  // ZListView::
  virtual void AssertDependantViews();
  virtual void CopyListViews(OptoStructs::lpA_GlassView_t& v);

  virtual void Absorb_ListAdd(OptoStructs::ZGlassImg* newimg, OptoStructs::ZGlassImg* before);
  virtual void Absorb_ListRemove(OptoStructs::ZGlassImg* eximg);
  virtual void Absorb_ListRebuild();

  virtual void AssertListRnrs(const string& rnr);

  virtual void CollExpList();
  virtual void ExpandList();
  virtual void CollapseList();

  virtual void ShowListMembers();
  virtual void HideListMembers();

  float LeafPosition(FTW_Leaf* leaf);

#include "FTW_Branch.h7"
}; // endclass FTW_Branch

#endif
