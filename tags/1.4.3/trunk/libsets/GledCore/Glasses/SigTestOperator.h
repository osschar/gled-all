// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SigTestOperator_H
#define GledCore_SigTestOperator_H

#include <Glasses/Operator.h>

class SigTestOperator : public Operator
{
  MAC_RNR_FRIENDS(SigTestOperator);

private:
  void _init();

protected:
  Bool_t     bRaiseILL;  //! X{GS} 7 Bool()
  Bool_t     bRaiseBUS;  //! X{GS} 7 Bool()
  Bool_t     bRaiseSEGV; //! X{GS} 7 Bool()
  Bool_t     bRaiseFPE;  //! X{GS} 7 Bool()
  Double_t   fResult;       //!

public:
  SigTestOperator(const Text_t* n="SigTestOperator", const Text_t* t=0);
  virtual ~SigTestOperator();

  virtual void Operate(Operator::Arg* op_arg);

#include "SigTestOperator.h7"
  ClassDef(SigTestOperator, 1);
}; // endclass SigTestOperator

#endif
