// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_SaturnObserver_H
#define GledCore_SaturnObserver_H

#include <Glasses/Operator.h>

class SaturnObserver : public Operator {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(SaturnObserver);

private:
  void _init();

protected:
  SaturnInfo*	mTarget;	// X{GS} L{}

public:
  SaturnObserver(const Text_t* n="SaturnObserver", const Text_t* t=0) : Operator(n,t) { _init(); }

  virtual void Operate(Operator::Arg* op_arg)	 throw(Operator::Exception);

#include "SaturnObserver.h7"
  ClassDef(SaturnObserver, 1)
}; // endclass SaturnObserver

GlassIODef(SaturnObserver);

#endif
