// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//________________________________________________________________________
// Mover
//
// Translates and/or rotates mNode (if not null) in its local frame.
// mMi ~ index of Move axis (1~x,2~y,3~z),
// mMa ~ amount of displacement;
// mRi, mRj ~ indices determining the Rotation plane,
// mRa ~ amount of angular displacement (in radians).
// If mMa!=0 translates; if mRa!=0 rotates.
//________________________________________________________________________

#include "Mover.h"

ClassImp(Mover)

void Mover::_init()
{
  mMi = mRi = mRj = 0;
  mMa = mRa = 0;
}

/**************************************************************************/

void Mover::Operate(Operator::Arg* op_arg) throw(Operator::Exception)
{
  Operator::PreOperate(op_arg);
  if(mNode) {
    if(mMa != 0) {
      OP_EXE_OR_SP_MIR(mNode, MoveLF, mMi, mMa);
    }
    if(mRa != 0) {
      OP_EXE_OR_SP_MIR(mNode, RotateLF, mRi, mRj, mRa);
    }
  }
  Operator::PostOperate(op_arg);
}

#include "Mover.c7"
