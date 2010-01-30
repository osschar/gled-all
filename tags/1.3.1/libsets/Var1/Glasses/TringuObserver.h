// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringuObserver_H
#define Var1_TringuObserver_H

#include <Glasses/ZNode.h>

class TringuObserver : public ZNode
{
  MAC_RNR_FRIENDS(TringuObserver);

private:
  void _init();

protected:

public:
  TringuObserver(const Text_t* n="TringuObserver", const Text_t* t=0);
  virtual ~TringuObserver();

#include "TringuObserver.h7"
  ClassDef(TringuObserver, 1);
}; // endclass TringuObserver

#endif
