// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// WSWalker
//
// Moves mNode as instructed. mNode should be a child of mSeed (this
// is not checked anywhere). This restriction could be removed.
//
// 1) Direct SetTime
// 2) Operator loop   -> takes time from Eventor stack
// 3) TimeMakerClient -> as given by the argument

#include "WSWalker.h"
#include "WSWalker.c7"
#include <Glasses/Eventor.h>

ClassImp(WSWalker)

/**************************************************************************/

void WSWalker::_init()
{
  bLoop = false;
}

/**************************************************************************/

void WSWalker::set_trans(Double_t t)
{
  if(mNode != 0 && mSeed != 0) {
    ZTrans lcf;
    mSeed->TransAtTime(lcf, t, bLoop, true);
    GLensWriteHolder node_wrlck(*mNode);
    mNode->SetTrans(lcf);
  } 
}

/**************************************************************************/

void WSWalker::SetTime(Double_t t)
{
  set_trans(t);
}

/**************************************************************************/

void WSWalker::Operate(Operator::Arg* op_arg) throw(Operator::Exception)
{
  Operator::PreOperate(op_arg);
  set_trans(op_arg->fEventor->GetEventTime());
  Operator::PostOperate(op_arg);
}

/**************************************************************************/

void WSWalker::TimeTick(Double_t t, Double_t dt)
{
  set_trans(t);
}
