// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Sleeper.h"
#include <TSystem.h>

ClassImp(Sleeper)

void Sleeper::Operate(Operator::Arg* op_arg) throw(Operator::Exception)
{
  gSystem->Sleep(mMSec);
}

#include "Sleeper.c7"
