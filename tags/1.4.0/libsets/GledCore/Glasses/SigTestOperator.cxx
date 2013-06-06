// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SigTestOperator.h"
#include "SigTestOperator.c7"

#include <Gled/GThread.h>

// SigTestOperator

//______________________________________________________________________________
//
//

ClassImp(SigTestOperator);

//==============================================================================

void SigTestOperator::_init()
{
  bRaiseILL = bRaiseBUS = bRaiseSEGV = bRaiseFPE = false;
  fResult = 0;
}

SigTestOperator::SigTestOperator(const Text_t* n, const Text_t* t) :
  Operator(n, t)
{
  _init();
}

SigTestOperator::~SigTestOperator()
{}

//==============================================================================

void SigTestOperator::Operate(Operator::Arg* op_arg)
{
  PreOperate(op_arg);

  if (bRaiseILL)
  {
    {
      GLensWriteHolder wrlck(this);
      SetRaiseILL(false);
    }
    // This just causes segv now :(
    // unsigned char insn[8] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    // void (*function)() = (void (*)()) insn;
    // function();
    GThread::Self()->Kill(GThread::SigILL);
  }

  if (bRaiseBUS)
  {
    {
      GLensWriteHolder wrlck(this);
      SetRaiseBUS(false);
    }
    // This does not "work".
    // char     *cptr = (char*) malloc(2*sizeof(Long64_t));
    // Long64_t *iptr = (Long64_t*) (cptr + 3);
    // *iptr = 42;
    GThread::Self()->Kill(GThread::SigBUS);
  }

  if (bRaiseSEGV)
  {
    {
      GLensWriteHolder wrlck(this);
      SetRaiseSEGV(false);
    }
    // This causes ill on mac.
    // TObject* obj = 0;
    // obj->Print();
    GThread::Self()->Kill(GThread::SigSEGV);
  }

  if (bRaiseFPE)
  {
    {
      GLensWriteHolder wrlck(this);
      SetRaiseFPE(false);
    }
    Double_t zero = 0;
    fResult = 124343.5465 / zero;
  }

  PostOperate(op_arg);
}

