// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_Eventor_H
#define Gled_Eventor_H

#include <Glasses/Operator.h>
#include <Glasses/SaturnInfo.h>
class Mountain;
class GThread;

class Eventor : public Operator {
private:
  void		_init();

protected:
  Double_t	mTotalTime;	//  in s; X{GS} 7 ValOut()
  Double_t	mRunTime;	//  in s; X{GS} 7 ValOut()

  Int_t		mBeatsToDo;	//  X{GS} 7 Value(-range=>[-1,1000000000,1])
  Int_t		mBeatsDone;	//  X{GS} 7 ValOut(-range=>[0,1000000],-join=>1)
  Int_t		mLocBeatsDone;	//! X{GS} 7 ValOut(-range=>[0,1000000]) local value
  Int_t		mStampInterval;	//  X{GS} 7 Value(-range=>[0,1000000,1],-join=>1)
  Int_t		mInterBeatMS;	//  X{GS} 7 Value(-range=>[0,1000000,1])

  // Thread properties
  SaturnInfo*	mHost;		//  X{GE} L{}

  Bool_t	bMultix;	//  X{GS} 7 Bool(-join=>1)
  Bool_t	bSignalSafe;	//  X{GS} 7 Bool()
  Bool_t	bContinuous;	//  X{GS} 7 Bool(-join=>1)
  Bool_t	bUseDynCast;	//  X{GS} 7 Bool()

  // Thread running status
  Bool_t	bRunning;	//  X{GS} 7 BoolOut(-join=>1)
  Bool_t	bSuspended;	//  X{GS} 7 BoolOut()
  Bool_t	bPerforming;	//! X{GS} 7 BoolOut(-join=>1)
  Bool_t	bXMultix;	//  X{GS} 7 BoolOut()

  virtual void AdEndarkenment();
  virtual void AdUnfoldment();

public:
  Eventor(const Text_t* n="Eventor", const Text_t* t=0) :
    Operator(n, t) { _init(); }

  Mountain* GetChaItOss();

  virtual Operator::Arg* PreDance(Operator::Arg* op_arg=0);
  virtual void		PostDance(Operator::Arg* op_arg);

  virtual void	PreBeat(Operator::Arg* op_arg)	 throw(Operator::Exception);
  virtual void	PostBeat(Operator::Arg* op_arg)	 throw(Operator::Exception);

  // On Change of running state
  virtual void	OnStart(Operator::Arg* op_arg);
  virtual void	OnSuspend(Operator::Arg* op_arg);
  virtual void	OnResume(Operator::Arg* op_arg);
  virtual void	OnExit(Operator::Arg* op_arg);

  // On Exception exit
  virtual void	OnContinue(Operator::Arg* op_arg, Operator::Exception& op_exc);
  virtual void	OnWait(Operator::Arg* op_arg, Operator::Exception& op_exc);
  virtual void	OnStop(Operator::Arg* op_arg, Operator::Exception& op_exc);
  virtual void	OnBreak(Operator::Arg* op_arg, Operator::Exception& op_exc);

  virtual void	Start();	// X{E} 7 MButt(-join=>1)
  virtual void	Stop();		// X{E} 7 MButt()
  virtual void	Suspend();	// X{E} 7 MButt(-join=>1)
  virtual void	Resume();	// X{E} 7 MButt()
  virtual void	Reset();	// X{E} 7 MButt(-join=>1)
  virtual void	Cancel();	// X{E} 7 MButt()

  void SetHost(SaturnInfo* host);

#include "Eventor.h7"
  ClassDef(Eventor, 1)
}; // endclass Eventor

GlassIODef(Eventor);

#endif
