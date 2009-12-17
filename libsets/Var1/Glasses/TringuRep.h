// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringuRep_H
#define Var1_TringuRep_H

#include <Glasses/ZNode.h>

class TringuRep : public ZNode
{
  MAC_RNR_FRIENDS(TringuRep);

private:
  void _init();

protected:

public:
  TringuRep(const Text_t* n="TringuRep", const Text_t* t=0);
  virtual ~TringuRep();

#include "TringuRep.h7"
  ClassDef(TringuRep, 1);
}; // endclass TringuRep

#endif
