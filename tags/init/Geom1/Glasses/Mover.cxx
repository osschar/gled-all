// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Mover.h"
#include <Gled/GledMirDefs.h>

#include "FL/gl.h"

#include <memory>

ClassImp(Mover)

void Mover::_init()
{
  mMi = mRi = mRj = 0;
  mMa = mRa = 0;
}

/**************************************************************************/

void Mover::Operate(Operator::Arg* op_arg) throw(Operator::Exception)
{
  if(!mNode) return;
  if(mMa != 0) {
    OP_EXE_OR_SP_MIR(mNode, MoveLF, mMi, mMa);
  }
  if(mRa != 0) {
    OP_EXE_OR_SP_MIR(mNode, RotateLF, mRi, mRj, mRa);
  }
}

#include "Mover.c7"
