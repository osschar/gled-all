// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SaturnObserver_H
#define GledCore_SaturnObserver_H

#include <Glasses/Operator.h>
class SaturnInfo;

class SaturnObserver : public Operator
{
  MAC_RNR_FRIENDS(SaturnObserver);

private:
  void _init();

protected:
  ZLink<SaturnInfo>	mTarget;	// X{gS} L{}

public:
  SaturnObserver(const Text_t* n="SaturnObserver", const Text_t* t=0) : Operator(n,t) { _init(); }

  virtual void Operate(Operator::Arg* op_arg);

#include "SaturnObserver.h7"
  ClassDef(SaturnObserver, 1);
}; // endclass SaturnObserver


#endif
