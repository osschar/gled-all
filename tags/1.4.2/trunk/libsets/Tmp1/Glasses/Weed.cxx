// $Id: Glass_SKEL.cxx 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Weed.h"
#include "Weed.c7"
#include "TMath.h"
#include <Glasses/ProductionRule.h>

// Weed

//______________________________________________________________________________
//
//

ClassImp(Weed);

//==============================================================================

void Weed::_init()
{
  bUseDispList = true;
}

Weed::Weed(const Text_t* n, const Text_t* t) :
ZNode(n, t),
mExpression(0),
mLength(0),
mLevel(0),
mAngle(TMath::PiOver2()),
mLineColor(1, 1, 0),
mLeafColor(1, 0, 1),
mFlowerColor(1, 1, 1)
{
  _init();
}

Weed::~Weed()
{}

//==============================================================================

char* Weed::ExpandString(char* oldExp, int oldSize, char* newExp, int level)
{
  ZVector &R = **(mRules);   
  int newIdx = 0;
  for (int i = 0; i < oldSize; ++i)
  {
    NameMap_i it  = mNameMap.find(oldExp[i]);
    if (it != mNameMap.end())
    {
      ProductionRule* pr = static_cast<ProductionRule*>(R[it->second]);
      TString rule = pr->GetRule(); 
      for (int k = 0; k < rule.Length(); ++k)
      {
        newExp[newIdx] = rule[k];
        ++newIdx;
      } 
    }
    else { 
      newExp[newIdx] = oldExp[i];
      ++newIdx;
    }
  }
  
  char* res = newExp;
  
  if (level < mLevel)
  {
    res = ExpandString(newExp, newIdx, oldExp, ++level);
  }

  return res;
}

//-------------------------------------------------------------------------------

void Weed::Produce()
{
  static const Exc_t _eh("Weed_GL_Rnr::Triangulate ");
  mNameMap.clear();
  
  ZVector &R = **(mRules);
  Int_t nRules = R.Size()+1; // children + non-reproductive characters
  
  // init matrix
  std::vector< std::vector<int> > prodMatrix(nRules);
  for (Int_t i = 0; i < nRules; ++i)
    prodMatrix[i].resize(nRules);
  
  // cache child name to rule idx
  AList::Stepper<ZGlass> ms(*mRules);
  Int_t idx = 0;
  while(ms.step())
    mNameMap[ms->GetName()[0]] = idx++;
  mNameMap[' '] = nRules-1; // space present any non reproductive character
  
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
  
  if (gDebug > 1)
  {
    ISdebug(1, _eh + "printing production matrix ...");
    for (int i=0; i<nRules; i++)
    {
      printf("rule %d:", i);
      for (int k =0; k<nRules; k++)
        printf("%d ", prodMatrix[i][k]);
      printf("\n");
    }
  }
  
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
  
  if (gDebug > 1)
  {
    ISdebug(1, _eh +"printing begin state vector ...");
    for (int i = 0; i < nRules; i++)
      printf("%d ", stateVec[i]);
    printf("\n");
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
  mLength = 0;
  for (int i = 0; i< nRules; ++i)
    mLength += stateVec[i];
  ISdebug(1, _eh + GForm("size %d for level %d \n", mLength, mLevel));
  
  // free memory
  delete [] mExpression;
  mExpression = 0;
  
  char* oldExp = new char[mLength]; // TODO one less than one iteraton
  char* newExp = new char[mLength];
  strcpy(oldExp, mStart.Data());
  char* res = ExpandString(oldExp,  mStart.Length(), newExp, 1);
  if (res == newExp)
    delete [] oldExp;
  else
    delete [] newExp;
  
  mExpression = res;
}


void Weed::DumpInfo() const 
{
  printf("Expression: \n");
  printf("%s\n\n", mExpression);
}
