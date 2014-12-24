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
mLateralAngle(70),
mAColor(1, 0, 1),
mBColor(0, 1, 1),
mLColor(1, 0, 0)
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



void DibotryoidHerb::DumpList(Segments_t& l)  
{
  char commandCol[13];
  
  char commandDef[13];
  
	/* Command is the control command to the terminal */
	//sprintf(command, "%c[%d;%d;%dm", 0x1B, 5, 33, 36); //"\033[5,33;36mboo"
	sprintf(commandCol, "%c[%d;%dm", 0x1B, 22, 33); //"\033[5,33;36mboo"
	sprintf(commandDef, "%c[%d;%dm", 0x1B, 22, 0); //"\033[5,33;36mboo"
  
  
  bool sideBranch = false;
  int nstack = 0;
  int n = (int)l.size();
  // printf("Expression[%d]: \n", (int)slist.size());
  for (int i=0; i< n; ++i)
  {
    if (l[i].mType == '[') nstack++;
    if (l[i].mType == ']') nstack--;
    
    if (nstack == 1 && l[i+1].mType == '+')
    {
      sideBranch = true;//\033[22;31mboo
      printf("%s", commandCol);
    }
    
    
    printf("%c", l[i].mType);
    if(l[i].mType == ']' && sideBranch && nstack == 0)
    {
      sideBranch = false;
      printf("%s", commandDef);
    }
      
   // if ((*i).mType == 'I') printf("(%d, %d)", (*i).mParam1, (*i).mParam2);
  }      
  printf(" \n");
  
  if (nstack) printf("!!!!!!!!!!!!!!!!!!stack %d \n", nstack);
}

void DibotryoidHerb::SegmentStepTime(Segments_i oldRef, Segments_t& in, Segments_t& out)
{  
  out.s('b', 1, 1);
  out.s('&', mLateralAngle);  
  out.s('+', mLateralAngle);  
  out.s('/', 180);

  Segment& seg = *oldRef;
  switch (seg.mType)
  {
      
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    case 'a': 
    {
      
      Segments_i ngi = NeighbourBack(in, oldRef);
      
      //   if (ng.mType == 'I' && ng.mParam1 == ng.mParam2)
      if (ngi != in.end() && (*ngi).mType == 'I' && (*ngi).mParam1 > 0)
      {
        // flowering
       // printf("'a'-> new flower, neighbour I (%d %d) plastocron %d\n",  ng.mParam1,  ng.mParam1, ng.mParam1);
        out.x('I', 0, -1).x("[L][+b]B");
        break;
      }
      
      if (seg.mParam1 < mPlastocronMainAxis)
      {
        // printf("a accumulate plastocron \n");
        out.x('a', seg.mParam1 + 1, seg.mParam2);
        break;
      }
      
      if (seg.mParam1 == mPlastocronMainAxis )
      {
        // build plain nodes
        if (seg.mParam2 <mCreateBranchDelay)
        {
          out.x('I', 0, -1).x("[L]").x('a', 1, seg.mParam2 + 1);
        } 
        else 
        {
          // building subranches
          out.x('I', 0, -1).x("[L][+b]/").x('a', 1, seg.mParam2);
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
      Segments_i ni = NeighbourBack(in, oldRef, 0, false); 
      if (seg.mParam1 == mPlastocronLateralAxis)
      {
       // printf("'b'-> new leaf segment : %d == %d\n", seg.mParam1, mPlastocronLateralAxis);        
        out.x('I', 0, -2).x("[L]").x('b', 1, seg.mParam2 + 1);
      }
      else if (ni != in.end() && (*ni).mType == 'I' && (*ni).mParam1 > 0/* == mSignalDelayLateralAxis*/)
      {
          //printf("'b'-> new lateral flower, neighbour I (%d %d) plastocron %d\n",  (*ni).mParam1,  seg.mParam1, seg.mParam1);
          out.x('I', 0, -2).x("[L]B");
          //out.x("B");
      }
      else if (seg.mParam1 < mPlastocronLateralAxis)
      {
      //  printf("'b'-> accumulate plastocron \n");
        out.x(seg.mType, seg.mParam1 + 1);
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
      int signalDelay = (seg.mParam2 == -1) ? mSignalDelayMainAxis : mSignalDelayLateralAxis;
      if (seg.mParam1 > 0 && seg.mParam1 < signalDelay)
      {
        out.x('I', seg.mParam1 + 1, seg.mParam2);
        break;
      }
      
      int stackOff = 0;
      if (seg.mParam2 == -2) // lateral brach
      {
        Segments_i ib = oldRef; --ib;
        if ((*ib).mType == '+') {
          stackOff = -1;
          // printf("xxxxxxxxxxxxxxxxxx\n");
        }
      }
      Segments_i nIt = NeighbourBack(in, oldRef, stackOff, false);  
      // printf("search neighbour for I(%d, %d) __ %c \n", seg.mParam1, seg.mParam2, (*nIt).mType );
      if (nIt != in.end())
      {
        
        if ((*nIt).mType == 'S' && (*nIt).mParam1 == mSendSignalDelay)
        {
          out.x('I', 1, seg.mParam2);
          break;
        }
        
        if ( (*nIt).mType == 'I')
        {
          int ngDelay = ((*nIt).mParam2 == -1) ? mSignalDelayMainAxis : mSignalDelayLateralAxis;
          if ((*nIt).mParam1 == ngDelay && seg.mParam1 == 0)
          {
            out.x('I', 1, seg.mParam2);
            break;
          }
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