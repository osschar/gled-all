// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_Mover_H
#define Geom1_Mover_H

#include <Glasses/Operator.h>
#include <Glasses/ZNode.h>

class Mover : public Operator {
private:
  void _init();

protected:
  ZNode*	mNode;	// X{GS} L{l}

  UInt_t	mMi;	// X{GS} 7 Value(-range=>[0,3,1], -width=>2, -join=>"true")
  Real_t	mMa;	// X{GS} 7 Value(-range=>[-10,10,1,1000])
  UInt_t	mRi;	// X{GS} 7 Value(-range=>[0,3,1], -width=>2, -join=>"true")
  UInt_t	mRj;	// X{GS} 7 Value(-range=>[0,3,1], -width=>2, -join=>"true")
  Real_t	mRa;	// X{GS} 7 Value(-range=>[-10,10,1,1000], -join=>"true")

public:
  Mover(Text_t* n="Mover", Text_t* t=0) : Operator(n,t), mNode(0) { _init(); }
  Mover(ZNode* m, Text_t* n="Mover", Text_t* t=0) :
    Operator(n,t), mNode(m) { _init(); }

  // virtuals
  virtual void Operate(Operator::Arg* op_arg) throw(Operator::Exception);

#include "Mover.h7"
  ClassDef(Mover, 1)
}; // endclass Mover

GlassIODef(Mover);

#endif
