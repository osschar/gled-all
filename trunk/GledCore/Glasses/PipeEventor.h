// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_PipeEventor_H
#define GledCore_PipeEventor_H

#include <Glasses/Eventor.h>

class PipeEventor : public Eventor {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(PipeEventor);

private:
  void _init();

protected:

  TString	mCommand;	// X{GS} 7 Textor();
  FILE*		mPipe;		//!

  UInt_t	mWaitTimeMS;	// X{GS} 7 Value(-range=>[1,1000000,1,1]);
  
  GCondition	mSendCond;	//!

  list<TString>	mPending;	//!

  void feed_commands();

public:
  PipeEventor(const Text_t* n="PipeEventor", const Text_t* t=0)
    : Eventor(n,t), mSendCond(GMutex::recursive)
  { _init(); }

  virtual void OnStart(Operator::Arg* op_arg);
  virtual void OnExit(Operator::Arg* op_arg);

  virtual void Operate(Operator::Arg* op_arg) throw(Operator::Exception);

  void PostCommand(TString& command);	// X{E}
  void ClearPendingCommands();		// X{E}

#include "PipeEventor.h7"
  ClassDef(PipeEventor, 1)
}; // endclass PipeEventor

GlassIODef(PipeEventor);

#endif
