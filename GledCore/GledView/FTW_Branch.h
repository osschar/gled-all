// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_Branch_H
#define GledCore_FTW_Branch_H

#include <Eye/OptoStructs.h>
#include "FTW_Leaf.h"
#include "FTW.h"

class FTW_Branch : public FTW_Leaf
{
 private:
  list<FTW_Leaf*>	mLeaves;
  TimeStamp_t		mLeavesTimeStamp;

  void build_leaves(int insert_pos);
  void wipe_leaves();

  void insert_leaf(Ray& ray, list<FTW_Leaf*>::iterator pos);
  void remove_leaf(list<FTW_Leaf*>::iterator pos);

  AList::ElRep beta_elrep(Ray& ray);
  AList::ElRep gamma_elrep(Ray& ray);
  list<FTW_Leaf*>::iterator leaf_pos(AList::ElRep elrep);

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

  virtual void AbsorbRay(Ray& ray);

  virtual void CollExpList();
  virtual void ExpandList();
  virtual void CollapseList();
  virtual void CopyListLeaves(list<FTW_Leaf*>& list);

  virtual void ShowListMembers();
  virtual void HideListMembers();

  float LeafPosition(FTW_Leaf* leaf);

#include "FTW_Branch.h7"
}; // endclass FTW_Branch

#endif
