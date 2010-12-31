// $Id: Glass_SKEL.cxx 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GrowingPlant.h"
#include "GrowingPlant.c7"

// GrowingPlant

//______________________________________________________________________________
//
//


ClassImp(GrowingPlant);


void GrowingPlant::_init()
{}

GrowingPlant::GrowingPlant(const Text_t* n, const Text_t* t) :
ZNode(n, t),
mLevel(1),
mLineColor(1, 1, 0)
{
  _init();
  // bUseDispList  = true;
}

GrowingPlant::~GrowingPlant()
{}

void
GrowingPlant::SegmentListStepTime(SegmentList& oldExp, SegmentList& newExp, int level)
{
  ++level; 
  // printf("-----------------------------growing plant lsit step time for level %d \n", level);
  // int idx = 0;
  for (Segments_i it = oldExp.begin(); it != oldExp.end(); ++it)
  {
    // printf("[%d] segment %c (%d, %d) step time \n", idx++, (*it).mType, (*it).mParam1, (*it).mParam2);
    SegmentStepTime(it, newExp);
  }
  oldExp.swap(newExp);
  newExp.clear();
  
  if (level < mLevel)
  {
    
    mOldSegments = oldExp;
    mNewSegments = newExp;
    SegmentListStepTime(oldExp, newExp, level);  
  }
}

void
GrowingPlant::Produce()
{
  // calback from triangulate method
  
  mSegments.clear();

  for (Segments_i it = mStart.begin(); it != mStart.end(); ++it)
    mSegments.push_back(*it);
  
  // create starting expresion from start rule and init paramters
  SegmentList refExp;
  mOldSegments = mSegments;
  mNewSegments = refExp;
  SegmentListStepTime(mSegments, refExp, 1);
}


void GrowingPlant::StepPlus()  
{
  SetLevel(++mLevel);
}

void GrowingPlant::StepMinus()  
{
  SetLevel(--mLevel);
}

void GrowingPlant::DumpInfo()  
{
  // printf("Expression[%d]: \n", (int)mSegments.size());
  for (Segments_i i = mSegments.begin(); i != mSegments.end(); ++i)
  {
    printf("%c", (*i).mType);
  }      
  printf(" \n");
}

//=============================================================================
GrowingPlant::SegmentList::SegmentList():
mChunkSize(1000)
{
  reserve(1000);
}

GrowingPlant::SegmentList&
GrowingPlant::SegmentList::s(char t, int v1, int v2)
{
  // Set defualt parameters. 
  // printf("sed defults %c (%d, %d) \n", t, v1, v2);
  for (GrowingPlant::Segments_i i = mDefaults.begin(); i != mDefaults.end(); ++i)
  {
    if ((*i).mType == t)
    {
      (*i).mParam1 = v1;
      (*i).mParam2 = v2;
      return *this;
    }
  }
  
  mDefaults.push_back(Segment(t, v1, v2));
  return *this;
}

GrowingPlant::SegmentList&
GrowingPlant::SegmentList::x(char t, int p1, int p2)
{
  push_back(Segment(t, p1, p2));
  //printf("add char/value %c %d \n", t, v);
  return *this;
}

GrowingPlant::SegmentList&
GrowingPlant::SegmentList::x(const char* nodes)
{
  for (size_t i = 0, e = strlen(nodes); i < e; ++i)
  {
    int param1 = -1;
    int param2 = -1;
    for  (GrowingPlant::Segments_i d = mDefaults.begin(); d != mDefaults.end(); ++d)
    {
      if ((*d).mType == nodes[i])
      {
        param1 = (*d).mParam1;
        param2 = (*d).mParam2;
        break;
      }
    }
    push_back(Segment(nodes[i], param1, param2));
  }
  
  return *this;
}
