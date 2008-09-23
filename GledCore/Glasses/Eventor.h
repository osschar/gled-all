// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_Eventor_H
#define GledCore_Eventor_H

#include <Glasses/Operator.h>
#include <Glasses/SaturnInfo.h>
class Mountain;
class GThread;

class Eventor : public Operator
{
private:
  void		_init();

protected:
  Double_t	mTotalTime;	//  in s; X{gS} 7 ValOut()
  Double_t	mRunTime;	//  in s; X{gS} 7 ValOut()

  Int_t		mBeatsToDo;	//  X{gS} 7 Value(-range=>[-1,1000000000,1])
  Int_t		mBeatsDone;	//  X{gS} 7 ValOut(-range=>[0,1000000],-join=>1)
  Int_t		mLocBeatsDone;	//! X{gS} 7 ValOut(-range=>[0,1000000]) local value
  Int_t		mStampInterval;	//  X{gS} 7 Value(-range=>[0,1000000,1],-join=>1)
  Int_t		mInterBeatMS;	//  X{gS} 7 Value(-range=>[0,1000000,1])

  // Thread properties
  ZLink<SaturnInfo> mHost;	//  X{gE} L{}

  Bool_t	bMultix;	//  X{gS} 7 Bool(-join=>1)
  Bool_t	bSignalSafe;	//  X{gS} 7 Bool()
  Bool_t	bContinuous;	//  X{gS} 7 Bool(-join=>1)
  Bool_t	bUseDynCast;	//  X{gS} 7 Bool()

  // Thread running status
  Bool_t	bRunning;	//  X{gS} 7 BoolOut(-join=>1)
  Bool_t	bSuspended;	//  X{gS} 7 BoolOut()
  Bool_t	bPerforming;	//! X{gS} 7 BoolOut(-join=>1)
  Bool_t	bXMultix;	//  X{gS} 7 BoolOut()

  virtual void AdEndarkenment();
  virtual void AdUnfoldment();

public:
  Eventor(const Text_t* n="Eventor", const Text_t* t=0) :
    Operator(n, t) { _init(); }

  Mountain* GetChaItOss();

  virtual Operator::Arg* PreDance(Operator::Arg* op_arg=0);
  virtual void           PostDance(Operator::Arg* op_arg);

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
  virtual void	OnBreak(Operator::Arg* op_arg, const TString& msg);

  virtual void	Start();	// X{E} 7 MButt(-join=>1)
  virtual void	Stop();		// X{E} 7 MButt()
  virtual void	Suspend();	// X{E} 7 MButt(-join=>1)
  virtual void	Resume();	// X{E} 7 MButt()
  virtual void	Reset();	// X{E} 7 MButt(-join=>1)
  virtual void	Cancel();	// X{E} 7 MButt()

  virtual void ResetRecursively(); // X{E} 7 MButt()

  void SetHost(SaturnInfo* host);


  //-------------------------------------
  // Time service for operators
  //-------------------------------------

public:
  enum EpochType_e  { ET_Manual, ET_DanceStart };
  enum TimeSource_e { TS_System, TS_IntStep };

  Double_t GetEventTime();
  void     PushEventTime(Double_t time);
  void     PopEventTime();

protected:
  Int_t          mEventID;      // X{GS} 7 Value()
  Double_t       mInternalTime; // X{GS} 7 Value()

  EpochType_e    mEpochType;    // X{GS} 7 PhonyEnum(-width=>8, -join=>1)
  TimeSource_e   mTimeSource;   // X{GS} 7 PhonyEnum(-width=>8)

  Double_t       mTimeEpoch;    // X{GS} 7 Value(-join=>1)
  Double_t       mTimeStep;     // X{GS} 7 Value(-range=>[-1e6,1e6,1,1000])

  list<Double_t> mTimeStack;    //!

public:

#include "Eventor.h7"
  ClassDef(Eventor, 1);
}; // endclass Eventor


#endif
