// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Mover.h"
#include "Mover.c7"
#include <Gled/GledOperatorDefs.h>

//________________________________________________________________________
//
// Translates and/or rotates mNode (if not null) in its local frame.
// mMi ~ index of Move axis (1~x,2~y,3~z),
// mMa ~ amount of displacement;
// mRi, mRj ~ indices determining the Rotation plane,
// mRa ~ amount of angular displacement (in radians).
// If mMa!=0 translates; if mRa!=0 rotates.
//________________________________________________________________________

ClassImp(Mover);

void Mover::_init()
{
  mMi = 1; mRi = 1; mRj = 2;
  mMa = mRa = 0;
  bMovParentFrame = bRotParentFrame = false;
}

/**************************************************************************/

void Mover::SetRotateParams(UInt_t i, UInt_t j, Float_t a)
{
  mRi = i; mRj = j; mRa = a;
  Stamp(FID());
}

/**************************************************************************/

void Mover::Operate(Operator::Arg* op_arg)
{
  Operator::PreOperate(op_arg);
  if(mNode != 0) {
    if(mMa != 0) {
      if(bMovParentFrame) {
	OP_EXE_OR_SP_MIR(mNode.get(), MovePF, mMi, mMa);
      } else {
	OP_EXE_OR_SP_MIR(mNode.get(), MoveLF, mMi, mMa);
      }
    }
    if(mRa != 0) {
      if(bRotParentFrame) {
	OP_EXE_OR_SP_MIR(mNode.get(), RotatePF, mRi, mRj, mRa);
      } else {
	OP_EXE_OR_SP_MIR(mNode.get(), RotateLF, mRi, mRj, mRa);
      }
    }
  }
  Operator::PostOperate(op_arg);
}
