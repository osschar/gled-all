// $Id: Glass_SKEL.h 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_SympodialTree_H
#define Tmp1_SympodialTree_H

#include <Glasses/MonopodialTree.h>

class SympodialTree : public MonopodialTree
{
  MAC_RNR_FRIENDS(SympodialTree);

private:
  
protected:
  
  virtual void ExpandRule(const Text_t* rule, TwoParam& parent, ParametricSystem::Segments_t& out);
  
public:
  SympodialTree(const Text_t* n="SympodialTree", const Text_t* t=0);
  virtual ~SympodialTree();

#include "SympodialTree.h7"
  ClassDef(SympodialTree, 1);
}; // endclass SympodialTree

#endif
