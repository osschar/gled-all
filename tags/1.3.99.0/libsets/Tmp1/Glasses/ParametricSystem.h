// $Id: Glass_SKEL.h 2089 2008-11-23 20:31:03Z matevz $

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_ParametricSystem_H
#define Tmp1_ParametricSystem_H

#include <Glasses/ZNode.h>
#include <Glasses/ZVector.h>
#include <Stones/ZColor.h>

struct TwoParam
{
  TwoParam(char type, float p1, float p2):mType(type), mParam1(p1), mParam2(p2) {}
  TwoParam(char type) : mType(type), mParam1(-1), mParam2(-1) {}

  TwoParam() {printf("empty !!!!!\n ");}
  virtual ~TwoParam(){}
  
  void Set(char t, float p1, float p2) { mType = t; mParam1 = p1; mParam2 = p2; }
  
  char   mType;
  float  mParam1;
  float  mParam2;  
};

//===============================================================================

class ParametricSystem : public ZNode
{
  MAC_RNR_FRIENDS(ParametricSystem);
protected:
  typedef std::vector<TwoParam>     Segments_t;  
  typedef Segments_t::iterator      Segments_i;  
  
private:  
  
  void _init();
  
  void SetupNameMap();
  int  EvalExpressionSize();          
protected:
  
  typedef std::map<char, int>       NameMap_t;
  typedef NameMap_t::iterator       NameMap_i;  
  
  Int_t          mLevel;      // X{GST} 7 Value(-range=>[1, 11, 1])
  TString        mStart;      // X{GST} 7 Textor()  
  ZLink<ZVector> mRules;      // X{GS}  L {}
  ZColor	       mLineColor;  // X{GSPT} 7 ColorButt()

  
  NameMap_t      mNameMap;
  Segments_t     mExpression;
 
  double         mRndWeight;  // X{GST} 7 Value(-range=>[0, 1, 1, 1000])
  
  virtual void ExpandExpression(Segments_t& oldExp, Segments_t& newExp, int level);

  virtual void ExpandRule(const Text_t* rule, TwoParam& parent, Segments_t& s) {};
  virtual void InitialiseExpression() {};

  Segments_t GetExpression() const { return mExpression; }
  
  void Produce();

public:
  ParametricSystem(const Text_t* n="ParametricSystem", const Text_t* t=0);
  virtual ~ParametricSystem();
   
  void DumpInfo();    // X{ED}  7 MButt()
     
#include "ParametricSystem.h7"
  ClassDef(ParametricSystem, 1);
}; // endclass ParametricSystem

#endif
