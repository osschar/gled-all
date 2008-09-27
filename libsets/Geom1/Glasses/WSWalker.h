// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_WSWalker_H
#define Geom1_WSWalker_H

#include <Glasses/Operator.h>
#include <Stones/ZTrans.h>
#include <Stones/TimeMakerClient.h>

class WSSeed;
class ZNode;

class WSWalker : public Operator, public TimeMakerClient
{
  MAC_RNR_FRIENDS(WSWalker);

private:
  void _init();

protected:
  ZLink<WSSeed>  mSeed; // X{GS} L{}
  ZLink<ZNode>   mNode; // X{GS} L{}

  Bool_t         bLoop; // X{GS} 7 Bool()

  void set_trans(Double_t t);

  // Local cache
  Double_t m_last_time;  //!
  ZTrans   m_last_trans; //!


public:
  WSWalker(const Text_t* n="WSWalker", const Text_t* t=0) :
    Operator(n,t) { _init(); }

  // Direct
  void SetTime(Double_t t);   // X{E} 7 MCWButt()

  // Operator
  virtual void Operate(Operator::Arg* op_arg);

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

#include "WSWalker.h7"
  ClassDef(WSWalker, 1) // Moves ZNode along a WeaverSymbol
}; // endclass WSWalker


#endif
