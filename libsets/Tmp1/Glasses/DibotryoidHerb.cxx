// $Id: Glass_SKEL.cxx 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "DibotryoidHerb.h"
#include "DibotryoidHerb.c7"

// DibotryoidHerb

//______________________________________________________________________________
//
//

ClassImp(DibotryoidHerb);

DibotryoidHerb::DibotryoidHerb(const Text_t* n, const Text_t* t) :
GrowingPlant(n, t),
mSendSignalDelay(13),
mCreateBranchDelay(3),
mPlastocronMainAxis(2),
mPlastocronLateralAxis(3),
mSignalDelayMainAxis(1),
mSignalDelayLateralAxis(1),
mLateralAngle(180)
{
  _init();
}

//==============================================================================

void DibotryoidHerb::_init()
{
  mStart.x('S', 0).x('a', 1, 0);
}

DibotryoidHerb::~DibotryoidHerb()
{}

//==============================================================================
GrowingPlant::Segments_i  DibotryoidHerb::NeighbourBack( Segments_i startIt)
{
  Segments_t& list = mOldSegments;
  if (list.empty())
    return list.begin();
    
  --startIt;
  int nStack = 0;  
  for (GrowingPlant::Segments_i i = startIt; i != mOldSegments.begin(); --i)
  {
    if ((*i).mType == '[')
    {
      --nStack;
    }
    else if ((*i).mType == ']')
    {
      ++nStack;  
    }
    else 
    {
      if (nStack <= 0 && ((*i).mType == 'I' ||  (*i).mType == 'S'))
        return i;
    }
  }

  printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>error\n");
  
  for (Segments_i i = list.begin(); i != list.end(); ++i)
  {
    printf("%c", (*i).mType);
  }      
  printf(" \n");
  
  return list.begin();
}

void DibotryoidHerb::SegmentStepTime(Segments_i oldRef, Segments_t& out)
{  
  out.s('b', 1, 1);
  out.s('&', 70);  
  out.s('/', mLateralAngle);

  Segment& seg = *oldRef;
  switch (seg.mType)
  {
      
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    case 'a': 
    {
      
      Segment& ng = *NeighbourBack(oldRef);
      
      //   if (ng.mType == 'I' && ng.mParam1 == ng.mParam2)
      if (ng.mType == 'I' && ng.mParam1 == ng.mParam2)
      {
        // flowering
       // printf("'a'-> new flower, neighbour I (%d %d) plastocron %d\n",  ng.mParam1,  ng.mParam1, ng.mParam1);
        out.x('I', 0, mSignalDelayMainAxis).x("[L][&b]B");
        break;
      }
      
      if (seg.mParam1 < mPlastocronMainAxis)
      {
        // printf("a accumulate plastocron \n");
        out.x("/").x('a', seg.mParam1 + 1, seg.mParam2);
        break;
      }
      
      if (seg.mParam1 == mPlastocronMainAxis )
      {
        // build plain nodes
        if (seg.mParam2 <mCreateBranchDelay)
        {
          out.x('I', 0, mSignalDelayMainAxis).x("[L]").x('a', 1, seg.mParam2 + 1);
        } 
        else 
        {
          // building subranches
          out.x('I', 0, mSignalDelayMainAxis).x("[L][&b]").x('a', 1, seg.mParam2);
        }
        break;
      }
      
      
      {
        printf("'a'-> should not happen, problem %d \n", seg.mParam1); 
      }
      
      break;
    } 
      
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    case  'b':
    {
      Segment& x = *NeighbourBack(oldRef); 
      if (seg.mParam1 == mPlastocronLateralAxis)
      {
       // printf("'b'-> new leaf segment : %d == %d\n", seg.mParam1, mPlastocronLateralAxis);        
        out.x('I', 0, mSignalDelayLateralAxis).x("[L]").x('b', 1, seg.mParam2 + 1);
      }
      else if (x.mType == 'I' && x.mParam1 > 0)
      {
        // printf("'b'-> new lateral flower, neighbour I (%d %d) plastocron %d\n",  x.mParam1,  x.mParam1, seg.mParam1);
        out.x('I', 1, mSignalDelayLateralAxis).x("[L]B");
      }
      else if (seg.mParam1 < mPlastocronLateralAxis)
      {
      //  printf("'b'-> accumulate plastocron \n");
        out.x(seg.mType, seg.mParam1 + 1, seg.mParam2 + 1);
      }
      else
      {
        printf("'b'-> should not happen, problem %d \n", seg.mParam1); 
      }

      break;
    }
      
    case 'S':
    {
      // signal delay
      // printf("signal delay S %d \n",  seg.mParam1 + 1);
      out.x('S', seg.mParam1 + 1, seg.mParam2);
      break;
    } 
      
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    case 'I':
    {
      // node
      if (seg.mParam1 > 0 && seg.mParam1 < seg.mParam2)
      {
        out.x('I', seg.mParam1 + 1, seg.mParam2);
        break;
      }
      
      Segments_i nIt = NeighbourBack(oldRef);  
      // printf("search neighbour for I(%d, %d) __ %c \n", seg.mParam1, seg.mParam2, (*nIt).mType );

      if ((*nIt).mType == 'S' && (*nIt).mParam1 == mSendSignalDelay)
      {
        out.x('I', 1, seg.mParam2);
        break;
      }
      
      
      if ((*nIt).mType == 'I')
      {
        // printf("compare params %d %d\n",  (*nIt).mParam1, (*nIt).mParam2);
        if ((*nIt).mParam1 == (*nIt).mParam2 && seg.mParam1 == 0)
        {
          out.x('I', 1, seg.mParam2);
          break;
        }
      }
      
      out.x(seg.mType, seg.mParam1, seg.mParam2);
      break;
      
    }
      
      /////////////////////////////////////////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////////////////////
    case 'B':
    {
      // flowering node
      out.x('I', 0, 1).x("[K]B");
      break;    
    } 
      
    default:
      // copy existing
      out.x(seg.mType, seg.mParam1, seg.mParam2);
      break;
  }
}