// $Id: Glass_SKEL.cxx 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GrowingPanicle.h"
#include "GrowingPanicle.c7"

// GrowingPanicle

//______________________________________________________________________________
//
//

ClassImp(GrowingPanicle);

//==============================================================================

void GrowingPanicle::_init()
{
  
  mStart.x('I', 20).x('F', 1).x('A', 0, 0);
}

GrowingPanicle::GrowingPanicle(const Text_t* n, const Text_t* t) :
GrowingPlant(n, t),
mLateralAngle(90),
mSColor(1, 1, 1),
mSSize(0.15),
mTColor(0, 0, 1),
mTSize(0.3),
mUColor(0, 1, 1),
mUSize(0.2)
{
  _init();
}

GrowingPanicle::~GrowingPanicle()
{}

//==============================================================================

void GrowingPanicle::SegmentStepTime(GrowingPlant::Segments_i ref, Segments_t& in, Segments_t& out)
{
  
  int delay = 2;
  //int sideLen = 1;
  //int mainLen = 3;
  
  out.s('/', 180);
  out.s('+', mLateralAngle);  
  out.s('A', 2, 0);
  out.s('O', 1, 1);
  
  Segment& seg   = *ref;

  //printf("step time %c \n", seg.mType);
  switch (seg.mType)
  {
    case'A': 
    {
      //  printf("case A \n");
      Segments_i back  = NeighbourBack(in, ref);
      if (back != in.end() && (*back).mType == 'S')
      { 
       // printf("A(%d, %d) create T \n", seg.mParam1, seg.mParam2);
        //if (1 seg.mParam2 ==0)
         out.x('T', 0).x("O");
        //else 
        //  out.x('A', seg.mParam1, seg.mParam2);
      }
      else if (seg.mParam1 > 0)
      {
        out.x('A', seg.mParam1 - 1, seg.mParam2);
      }
      else if (seg.mParam1 == 0)
      {
        out.x("[+G]").x('F', 1, seg.mParam2).x("/").x('A', delay, seg.mParam2);
      }
      break;
    }
      ///////////////////////////////////////////////////////////////////////////
     
    case 'F':
    {
      Segments_i back  = NeighbourBack(in, ref);
      if (back != in.end() && (*back).mType == 'S')
      { 
       //if (seg.mParam2 == 0)  printf("case F move signal S forward\n");
        out.x(seg.mType, seg.mParam1, seg.mParam2).x('S', 1, 1);
        break;
      }
      
      Segments_i front = NeighbourFront(in, ref, 0, false);
      if (front != in.end() && (*front).mType == 'T') 
      { 
        //if (1 || seg.mParam2 == 0) printf("T in front F(%d, %d) -> creaincrement growth potential\n", seg.mParam1, seg.mParam2);
        out.x('T', (*front).mParam1 + 1).x('F', 1, seg.mParam2).x('U',  (*front).mParam1 -1);
        break;
      } 
      
      // default
      out.x(seg.mType, seg.mParam1, seg.mParam2);
      break;
    }
      
      
      ///////////////////////////////////////////////////////////////////////////

    case 'G':
    {
      // printf("case G\n");
    
      Segments_i back  = NeighbourBack(in, ref, -1);
      if (back != in.end() && (*back).mType == 'U')
      { 
        // printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! side branch transform G -> FA\n");
         out.x('I', (*back).mParam1).x('F', 1, 1).x('A', delay, 1);
        // out.x('I', (*back).mParam1).x("FA");
        break;
      }
      
      out.x(seg.mType, seg.mParam1, seg.mParam2);
      break;
    }
      
      ///////////////////////////////////////////////////////////////////////////

    case 'I':
    {
     // printf("case I\n");
      if (seg.mParam1 == 0)
      {
      // printf("emit signal S \n");
        out.x('S', 0);
      }
      else
      {
        out.x('I', seg.mParam1 -1);
       // printf("I timeout\n");
      }
      break;
    }
    
      ///////////////////////////////////////////////////////////////////////////

    case 'S':
    case 'T':
    {
      break;
    }

    case 'O':
    {
      // change color and size of a bud
      out.x(seg.mType, seg.mParam1+1, seg.mParam2+1);
      break; 
    }
    default:
    {
      //printf("copy default %c\n", seg.mType);
      out.x(seg.mType, seg.mParam1, seg.mParam2);
    }
  }
}