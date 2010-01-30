// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Alice_FpeOperator_H
#define Alice_FpeOperator_H

#include <Glasses/Operator.h>

class FpeOperator : public Operator
{
  MAC_RNR_FRIENDS(FpeOperator);

private:
  void _init();

protected:
  Bool_t     bRaiseILL;  //! X{GS} 7 Bool()
  Bool_t     bRaiseBUS;  //! X{GS} 7 Bool()
  Bool_t     bRaiseSEGV; //! X{GS} 7 Bool()
  Bool_t     bRaiseFPE;  //! X{GS} 7 Bool()
  Double_t   fResult;       //!

public:
  FpeOperator(const Text_t* n="FpeOperator", const Text_t* t=0);
  virtual ~FpeOperator();

  virtual void Operate(Operator::Arg* op_arg);

#include "FpeOperator.h7"
  ClassDef(FpeOperator, 1);
}; // endclass FpeOperator

#endif
