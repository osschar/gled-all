// $Id: Glass_SKEL.cxx 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SympodialTree.h"
#include "SympodialTree.c7"

// SympodialTree

//______________________________________________________________________________
//
//

ClassImp(SympodialTree);

//==============================================================================


SympodialTree::SympodialTree(const Text_t* n, const Text_t* t) :
  MonopodialTree(n, t)
{
 // _init();
}

SympodialTree::~SympodialTree()
{}

//==============================================================================

void SympodialTree::ExpandRule(const Text_t* rule, TwoParam& parent, ParametricSystem::Segments_t& newExp)
{
  const static float widthDecrease = TMath::Sqrt(0.5f);
  
  printf("expand rule \n");
  bool firstStack = true;
  for (size_t k = 0; k < strlen(rule); ++k)
  {
    newExp.push_back(TwoParam(rule[k], parent.mParam1, parent.mParam2));  

    TwoParam& s = newExp.back();
    if (s.mType == ']')
    {
      firstStack = false;
      printf("change turn  SympodialTree\n");
    }
    
    if (s.mType == '&' || s.mType == '^')
    {
      s.mParam1 = firstStack ? mTrunkAngle : mLateralAngle;
    }
    else if (s.mType == '+' || s.mType == '-')
    {
      s.mParam1 = firstStack ? mTrunkAngle : mLateralAngle;
    }
    else if (s.mType == '\\' || s.mType == '/')
    {
      s.mParam1 = 180;
    }
    else if (s.mType == 'A' || s.mType == 'B')
    {
      float contraction = firstStack ? mTrunkContraction : mLateralContraction;
      //   printf("contractin %f \n", contraction);
      s.mParam1 *= contraction;
      s.mParam2 *= widthDecrease;
    }
  } 
  
}