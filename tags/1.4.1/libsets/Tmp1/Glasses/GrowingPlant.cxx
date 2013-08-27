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
//=================================================================================
void
GrowingPlant::SegmentListStepTime(SegmentList& oldExp, SegmentList& newExp, int level)
{
   bool debug = false;
  
  ++level; 
  //printf("duming new \n"); DumpList(newExp);
  int idx=0;
  for (Segments_i it = oldExp.begin(); it != oldExp.end(); ++it)
  {
    if (debug) printf("[%d] segment %c (%d, %d) step time \n", idx++, (*it).mType, (*it).mParam1, (*it).mParam2);
    SegmentStepTime(it, oldExp, newExp);
  }
  oldExp.swap(newExp);
  newExp.clear();
  
  if (level < mLevel)
  {
    if (debug)
    {
      printf("\n\n\n\ngrowing plant lsit step time for level %d sizes %d , %d \n", level, (int)oldExp.size(), (int)newExp.size());
      DumpList(oldExp);
    }
    SegmentListStepTime(oldExp, newExp, level);  
  }
}

void
GrowingPlant::Produce()
{
  // calback from triangulate method
   
  mSegments.clear();
  mSegments = mStart;
  
  // create starting expresion from start rule and init paramters
  if (mLevel > 1)
  {
    SegmentList refExp;
    SegmentListStepTime(mSegments, refExp, 1);
  }
   
  //  printf("finish Produce \n"); DumpInfo();

}


//==============================================================================

void GrowingPlant::DumpInfo()  
{
  // printf("Expression[%d]: \n", (int)mSegments.size());
  for (Segments_i i = mSegments.begin(); i != mSegments.end(); ++i)
  {
    printf("%c", (*i).mType);
  }      
  printf(" \n");
}

void GrowingPlant::DumpList(Segments_t& slist)  
{
  int nstack = 0;
  
  // printf("Expression[%d]: \n", (int)slist.size());
  for (Segments_i i = slist.begin(); i != slist.end(); ++i)
  {
    if ((*i).mType == '[') nstack++;
    if ((*i).mType == ']') nstack--;
    
    printf("%c", (*i).mType);
  }      
  printf(" \n");
  
  if (nstack) printf("!!!!!!!!!!!!!!!!!!stack %d \n", nstack);
}

//==============================================================================
//==============================================================================

GrowingPlant::Segments_i GrowingPlant::NeighbourFront( SegmentList& slist, Segments_i startIt, int depth, bool debug)
{
  static const Exc_t _eh("Weed_GL_Rnr::NeighbourFront ");  
  Segments_i startItOrig = startIt;
  ++startIt;
  
  if (debug) {
    printf("front neighbour-front search ");
    for (Segments_i i = startIt; i != slist.end(); ++i)
    {
      printf("%c", (*i).mType);
    }      
    printf(" \n");
  }
  
  int nStack = 0;  
  for (GrowingPlant::Segments_i i = startIt; i != slist.end(); ++i)
  {
    if ((*i).mType == '[')
    {
      --nStack;
    }
    else if ((*i).mType == ']')
    {
      if (depth == 0 && nStack == 0)  return slist.end();
      ++nStack;  
    }
    else 
    {
      if (nStack == depth && ((*i).mType == 'I' ||  (*i).mType == 'S' ||(*i).mType == 'F' ||(*i).mType == 'T' ||  (*i).mType == 'U' ))
      {
        if (debug) printf("found %c \n",  (*i).mType);
        return i;
      }
      else
      {
       if (debug) printf("skip %c \n",  (*i).mType);
      }
    }
  }

  ISerr(_eh + Form(" [%c] 'it == end.", (*startItOrig).mType));
  
  return slist.end();
}
//_____________________________________________________________________________________________

GrowingPlant::Segments_i GrowingPlant::NeighbourBack(SegmentList& slist, Segments_i startIt, int depth, bool debug)
{
  if (startIt == slist.begin()) return slist.end(); 
  
  static const Exc_t _eh("Weed_GL_Rnr::NeighbourBack ");
  char t = (*startIt).mType;
  if (debug)
  {
    printf("back neighbour-back search ");
    for (Segments_i j = startIt; j != slist.begin(); --j)
    {
      Segments_i i = j; 
      --i;
      printf("%c", (*i).mType);
    }      
    printf(" \n");
  }
  
  
  int nStack = 0;  
  for (GrowingPlant::Segments_i j = startIt; j != slist.begin(); --j)
  {
    
    Segments_i i = j;
    --i;
    if ((*i).mType == '[')
    {
      if (depth == 0 && nStack == 0) return slist.end();
      
      --nStack;
      if (debug) printf("increase stack \n");
    }
    else if ((*i).mType == ']')
    {
      ++nStack;  
      if (debug) printf("decrease stack \n");
    }
    else if (nStack == depth)
    {
      if ( (*i).mType == 'I' ||  (*i).mType == 'S'||(*i).mType == 'F'||(*i).mType == 'T' ||  (*i).mType == 'U' )
      {
        if (debug) printf("found back %c for %c\n", (*i).mType, t);
        return i;
      }
      else 
      {
        if (debug) printf("skip %c \n",  (*i).mType);
      }
    }
  }
  // printf( "found back %c not it == begin.\n",t );
  return slist.end();
}


//=============================================================================
void GrowingPlant::StepPlus()  
{
  SetLevel(++mLevel);
}

void GrowingPlant::StepMinus()  
{
  if (mLevel > 1)
    SetLevel(--mLevel);
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
