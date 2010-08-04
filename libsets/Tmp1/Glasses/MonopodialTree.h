// $Id: Glass_SKEL.h 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_MonopodialTree_H
#define Tmp1_MonopodialTree_H

#include <Glasses/ParametricSystem.h>

class MonopodialTree : public ParametricSystem
{
  MAC_RNR_FRIENDS(MonopodialTree);

private:
  void _init();

protected:
  float mTrunkContraction;      // X{GST} 7 Value(-range=>[0, 2, 1, 1000])
  float mLateralContraction;    // X{GST} 7 Value(-range=>[0, 2, 1, 1000])
  float mTrunkAngle;            // X{GST} 7 Value(-range=>[0, 360, 1])
  float mLateralAngle;          // X{GST} 7 Value(-range=>[0, 360, 1])
    
  float mStartLength;           // X{GE} 7 Value(-range=>[0, 1, 1, 1000])
  float mStartWidth;            // X{GE} 7 Value(-range=>[0, 1, 1, 1000])
    

  virtual void ExpandRule(const Text_t* rule, TwoParam& parent, ParametricSystem::Segments_t& out);
  virtual void InitialiseExpression();
  
public:
  MonopodialTree(const Text_t* n="MonopodialTree", const Text_t* t=0);
  virtual ~MonopodialTree();
  
  void SetStartWidth(float);  
  void SetStartLength(float);  
  
#include "MonopodialTree.h7"
  ClassDef(MonopodialTree, 1);
}; // endclass MonopodialTree

#endif
