// $Id: Glass_SKEL.cxx 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TernaryTree.h"
#include "TernaryTree.c7"

#include <Glasses/ProductionRule.h>
const float TernaryTree::s_Width_Increase = TMath::Sqrt(3);


// TernaryTree

//______________________________________________________________________________
//
//

ClassImp(TernaryTree);

//==============================================================================

void TernaryTree::_init()
{}

TernaryTree::TernaryTree(const Text_t* n, const Text_t* t) :
MonopodialTree(n, t),
mDivergenceAngle1(94),
mDivergenceAngle2(132),
mSusceptibility(0.27),
mTorsion(-1, 0, 0)
{
  _init();
  mTorsionNormalized = mTorsion;
}

TernaryTree::~TernaryTree()
{}

void TernaryTree::SetTorsion(Double_t x, Double_t y, Double_t z)
{
  mTorsion.Set(x,y,z);
  mTorsionNormalized = mTorsion;
  mTorsionNormalized.Normalize();
  mTorsionNormalized.Print();
  mStampReqTring = Stamp(FID());
}


void
TernaryTree::ResetTorsion()
{
  mTorsion.Set(-1, 0, 0);
  mTorsionNormalized = mTorsion;
}

void
TernaryTree::InitialiseExpression()
{
  assert(mExpression.empty());
  
  mExpression.push_back(TwoParam('!', mStartWidth, -1));
  mExpression.push_back(TwoParam('F', mStartLength, mStartWidth));
  mExpression.push_back(TwoParam('/', 45, 45));
  mExpression.push_back(TwoParam('A', mStartLength, mStartWidth));
  
  assert((Int_t)mExpression.size() == mStart.Length());
}

void
TernaryTree::ExpandExpression(Segments_t& oldExp, Segments_t& newExp, int level)
{
  ZVector &R = **(mRules);
  for (Segments_i seg = oldExp.begin(); seg != oldExp.end(); ++seg)
  {
    NameMap_i it  = mNameMap.find((*seg).mType);
    if (it != mNameMap.end())
    {
      ProductionRule* pr = static_cast<ProductionRule*>(R[it->second]);
      TString rule = pr->GetRule(); 
      ExpandRule(pr->GetRule(), *seg, newExp);
    }
    else 
    { 
      newExp.push_back(*seg);
      if ((*seg).mType == '!')
      {
        newExp.back().mParam1 = (*seg).mParam1 * TMath::Sqrt(3);
      }
      else if ((*seg).mType == 'F')
      {
        newExp.back().mParam1 *= mTrunkContraction; 
        newExp.back().mParam2 *= TMath::Sqrt(3);
      }
    }
  }
  
  if (level < mLevel)
  {
    oldExp.swap(newExp);
    newExp.clear();
    ExpandExpression(oldExp, newExp, ++level);  
  }
}


void
TernaryTree::ExpandRule(const Text_t* rule, TwoParam& parent, ParametricSystem::Segments_t& newExp)
{
  bool firstDiv = true;
  bool base = true;
  for (size_t k = 0; k < strlen(rule); ++k)
  {
    newExp.push_back(TwoParam(rule[k]));  
    TwoParam& s = newExp.back();
    
    if (parent.mType == 'A')
    {
      if (s.mType == '&' )
      {
        s.mParam1 = mTrunkAngle;
      }
      else if ( s.mType == '/')
      {
        s.mParam1 = firstDiv ? mDivergenceAngle1 : mDivergenceAngle2;
        firstDiv = false;
      }
      else if (s.mType == 'F')
      {
        s.mParam1 = mStartLength*0.25; 
        if (1 || base)
        {
          s.mParam2 = mStartWidth * TMath::Sqrt(3); 
        }
        else
        {
          s.mParam2 = mStartWidth; 
        }
        base = false;
      }
    }
  }
}
