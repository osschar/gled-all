// $Id: Glass_SKEL.cxx 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "ParametricSystem.h"
#include "ParametricSystem.c7"

#include "TMath.h"
#include <Glasses/ProductionRule.h>


// ParametricSystem

//______________________________________________________________________________
//
//

ClassImp(ParametricSystem);


void ParametricSystem::_init()
{}

ParametricSystem::ParametricSystem(const Text_t* n, const Text_t* t) :
ZNode(n, t),
mLevel(0),
mLineColor(1, 1, 0),
mRndWeight(0.0)
{
  _init();
  bUseDispList  = true;
}

ParametricSystem::~ParametricSystem()
{}

void
ParametricSystem::SetupNameMap()
{
  mNameMap.clear();
  
  ZVector &R = **(mRules);
  Int_t nRules = R.Size()+1; // children + non-reproductive characters
  
  // cache child name to rule idx
  AList::Stepper<ZGlass> ms(*mRules);
  Int_t idx = 0;
  while(ms.step())
    mNameMap[ms->GetName()[0]] = idx++;
  mNameMap[' '] = nRules-1; // space present any non reproductive character
}

int
ParametricSystem::EvalExpressionSize()
{
  static const Exc_t _eh("ParametricSystem::EvalExpressionSize ");
  
  ZVector &R = **(mRules);
  Int_t nRules = R.Size()+1; // children + non-reproductive characters
  
  // init matrix
  std::vector< std::vector<int> > prodMatrix(nRules);
  for (Int_t i = 0; i < nRules; ++i)
    prodMatrix[i].resize(nRules);
  
  // configure matrix
  AList::Stepper<ProductionRule> s(*mRules);
  Int_t ruleIdx = 0;
  while(s.step())
  {
    TString ruleString  = s->GetRule();
    for (int i = 0; i < ruleString.Length(); ++i)
    {
      NameMap_i it = mNameMap.find(ruleString[i]);
      if (it != mNameMap.end())
        prodMatrix[ruleIdx][it->second]++;
      else
        prodMatrix[ruleIdx][nRules-1]++;
    }
    ++ruleIdx;
  }
  
  // non-repoducable
  prodMatrix[nRules-1][nRules-1] = 1;
  
  // begin state
  std::vector<int> stateVec(nRules);
  for (int i=0; i< mStart.Length(); i++)
  {    
    NameMap_i it = mNameMap.find(mStart[i]);
    if (it != mNameMap.end())
      stateVec[it->second]++;
    else
      stateVec[nRules-1]++;
  }
  
  // get final state vector
  std::vector<int> newState(nRules);
  int level = 0;
  while (level < mLevel)
  {        
    // loop over rules
    for (int r = 0; r < nRules; ++r)
    {
      newState[r] = 0;
      for (int i = 0; i < nRules; ++i )
        newState[r] += stateVec[i] * prodMatrix[i][r];  
    }
    
    for (int i = 0; i < nRules; ++i)
    {
      stateVec[i] = newState[i];
    }
    ++level;
  }    
  
  // set final length
  int length = 0;
  for (int i = 0; i< nRules; ++i)
    length += stateVec[i];
  ISdebug(1, _eh + GForm("size %d for level %d \n", length, mLevel));
  
  return length;
}

void
ParametricSystem::ExpandExpression(Segments_t& oldExp, Segments_t& newExp, int level)
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
ParametricSystem::Produce()
{
  // free memory
  mExpression.clear();
  
  // eval memory usage, reserve space
  SetupNameMap();
  int l = EvalExpressionSize();
  mExpression.reserve(l);
  Segments_t refExp;
  refExp.reserve(l);
  
  InitialiseExpression();
  
  // create starting expresion from start rule and init paramters
  
  ExpandExpression(mExpression, refExp, 1);
  mExpression.swap(refExp);
  refExp.clear();
}

void ParametricSystem::DumpInfo()  
{
   printf("Expression[%d]: \n", (int)mExpression.size());
   for (Segments_i i = mExpression.begin(); i != mExpression.end(); ++i)
   {
      printf("%c", (*i).mType);
   }      
   printf(" \n");
}
