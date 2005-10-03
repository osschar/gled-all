// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_Sleeper_H
#define Geom1_Sleeper_H

#include <Glasses/Operator.h>

class Sleeper : public Operator {
protected:
  UInt_t	mMSec;	// X{GS} 7 Value(-range=>[0,1e9,1,1])

public:
  Sleeper(Text_t* n="Sleeper", Text_t* t=0) : Operator(n,t) {}
  Sleeper(UInt_t ms, Text_t* n="Sleeper", Text_t* t=0) :
    Operator(n,t), mMSec(ms) {}

  // virtuals
  virtual void Operate(Operator::Arg* op_arg) throw(Operator::Exception);

#include "Sleeper.h7"
  ClassDef(Sleeper, 1)
}; // endclass Sleeper


#endif
