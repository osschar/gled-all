// $Id: Glass_SKEL.h 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_GrowingPlant_H
#define Tmp1_GrowingPlant_H

#include <Glasses/ZNode.h>
#include <Stones/ZColor.h>
//===============================================================================

class GrowingPlant : public ZNode
{
  MAC_RNR_FRIENDS(GrowingPlant);
public:  
  struct Segment
  {
  public:
    Segment(char type, int p1 = -1, int p2 = -1) : mType(type), mParam1(p1), mParam2(p2) {}

    virtual ~Segment(){}
    
    void Set(char t, float p1, float p2) { mType = t; mParam1 = p1; mParam2 = p2; }
    
    char   mType;
    int    mParam1;
    int    mParam2;  
  };
  
  class SegmentList: public std::vector<Segment>
  {
  public:
    SegmentList();
    
    int mChunkSize;
    std::vector<Segment> mDefaults;
    
    SegmentList& s(char, int);
    SegmentList& x(char, int);
    SegmentList& x(const char*);
  };
  
  typedef SegmentList               Segments_t;  
  typedef SegmentList::iterator     Segments_i;  
  
private:  
  void _init();
  void SetupNameMap();        
  
protected:
  Int_t          mLevel;      // X{GST} 7 Value(-range=>[1, 50, 1])
  SegmentList    mSegments;
  SegmentList    mStart;
  
  ZColor	       mLineColor;    // X{GSPT} 7 ColorButt()
  ZColor	       mFlowerColor;  // X{GSPT} 7 ColorButt()
  
  virtual void SegmentStepTime(Segments_i olfIdx, Segments_t& out) {}; // abstract
  virtual void SegmentListStepTime(SegmentList& oldExp, SegmentList& newExp, int level);
 
public:
  GrowingPlant(const Text_t* n="GrowingPlant", const Text_t* t=0);
  virtual ~GrowingPlant();
  
  
  void StepPlus();    // X{ED}  7 MButt()
  void StepMinus();    // X{ED}  7 MButt()
  
  void DumpInfo();    // X{ED}  7 MButt()
  
  void Produce();
  
#include "GrowingPlant.h7"
  ClassDef(GrowingPlant, 1);
}; // endclass GrowingPlant

#endif
