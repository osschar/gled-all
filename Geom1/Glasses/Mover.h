// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_Mover_H
#define Geom1_Mover_H

#include <Glasses/Operator.h>
#include <Glasses/ZNode.h>

class Mover : public Operator
{
private:
  void _init();

protected:
  ZLink<ZNode>	mNode;	// X{gS} L{}

  UInt_t	mMi;	// X{gS} 7 Value(-range=>[1,3,1], -width=>2, -join=>1)
  Float_t	mMa;	// X{gS} 7 Value(-range=>[-10,10,1,1000000], -width=>8)
  UInt_t	mRi;	// X{gS} 7 Value(-range=>[1,3,1], -width=>2, -join=>1)
  UInt_t	mRj;	// X{gS} 7 Value(-range=>[1,3,1], -width=>2, -join=>1)
  Float_t	mRa;	// X{gS} 7 Value(-range=>[-10,10,1,1000000], -width=>8)

  Bool_t        bMovParentFrame; // X{GS} 7 Bool(-join=>1)
  Bool_t        bRotParentFrame; // X{GS} 7 Bool()

public:
  Mover(Text_t* n="Mover", Text_t* t=0) : Operator(n,t) { _init(); }
  Mover(ZNode* m, Text_t* n="Mover", Text_t* t=0) :
    Operator(n,t), mNode(m) { _init(); }

  void SetRotateParams(UInt_t i, UInt_t j, Float_t a); // X{E}

  // Operator:
  virtual void Operate(Operator::Arg* op_arg) throw(Operator::Exception);

#include "Mover.h7"
  ClassDef(Mover, 1);
}; // endclass Mover


#endif
