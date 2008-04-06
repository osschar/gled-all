// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_FTW_Branch_H
#define GledCore_FTW_Branch_H

#include <Eye/OptoStructs.h>
#include "FTW_Leaf.h"
#include "FTW.h"

class FTW_Branch : public FTW_Leaf
{
protected:
  typedef list<FTW_Leaf*>           lpFTW_Leaf_t;
  typedef list<FTW_Leaf*>::iterator lpFTW_Leaf_i;

  list<FTW_Leaf*>	mLeaves;
  TimeStamp_t		mLeavesTimeStamp;

  void build_leaves(int insert_pos);
  void wipe_leaves();

  FTW_Leaf* create_leaf(OptoStructs::ZGlassImg* img, const AList::ElRep& el_rep);
  void      insert_leaf(Ray& ray, lpFTW_Leaf_i pos);
  void      remove_leaf(lpFTW_Leaf_i pos);

  AList::ElRep beta_elrep(Ray& ray);
  AList::ElRep gamma_elrep(Ray& ray);
  lpFTW_Leaf_i leaf_pos(AList::ElRep elrep);

  bool			bLeavesCreated;
  bool			bListExpanded;	// mLeafs shown

  // Leaf Designations
  bool                  mLDShow;  // X{gs}
  bool                  mLDEdit;  // X{gs}
  int                   mLDWidth; // X{gs}
  int                   mLDNameMinWidth; // X{gs}

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

  virtual void RewidthNameBoxes();
  virtual void SpawnSeparatorDialog();

#include "FTW_Branch.h7"
}; // endclass FTW_Branch

#endif
