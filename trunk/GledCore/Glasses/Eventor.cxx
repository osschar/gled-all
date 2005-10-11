// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//________________________________________________________________________
// Eventor
//
// Base-class for glasses that wish to acquire their own threads.
//
//________________________________________________________________________

#include "Eventor.h"
#include "Eventor.c7"
#include <Ephra/Mountain.h>
#include <Glasses/ZKing.h>
#include <Glasses/ZQueen.h>
#include <Gled/GThread.h>
#include <Gled/GledMirDefs.h>


#include <signal.h>
#include <errno.h>

ClassImp(Eventor)

/**************************************************************************/

void Eventor::_init()
{
  mTotalTime = mRunTime = 0;
  mBeatsToDo = -1; mBeatsDone = 0;
  mStampInterval = 100; mInterBeatMS = 0;

  mHost = 0;

  bUseDynCast = true; bSignalSafe = false;
  bContinuous = true; bMultix = false;
  bRunning = bSuspended = bPerforming = bXMultix = false;

  mEventID      = 0;
  mInternalTime = 0;
  mEpochType    = ET_DanceStart;
  mTimeSource   = TS_System;
  mTimeStep     = 1;
  mTimeEpoch    = 0;
}

/**************************************************************************/

Mountain* Eventor::GetChaItOss()
{
  return mSaturn->GetChaItOss();
}

/**************************************************************************/

void Eventor::AdEndarkenment()
{
  // !!!! if thread running clomp it
}

void Eventor::AdUnfoldment()
{
  Operator::AdUnfoldment();
  mLocBeatsDone = mBeatsDone;

  // !!!! multix can change ... would better lock that!!!
  // anyway dislike the duplication in op_arg.
  // Too lazy to write the manual Set Methods?

  if(bXMultix && bRunning) {
    if(mHost == 0) {
      mHost = GetQueen()->GetKing()->GetSaturnInfo();
    }
    if(!bSuspended) {
      mSaturn->GetChaItOss()->Start(this);
    } else {
      mSaturn->GetChaItOss()->Start(this, true);
    }
  }
}

/**************************************************************************/
// Dance & Beat methods
/**************************************************************************/

Operator::Arg* Eventor::PreDance(Operator::Arg* op_arg)
{
  // Eventor::PreDance() is called as the first thing from Mountain::OperatorBeat().
  // It is supposed to instantiate and set Operator::Arg* op_arg to meaningfull
  // values and return it.
  // If you sub-class the Operator::Arg, make sure to also perform the
  // house-keeping tasks performed here or call this method with non-zero
  // op_arg.
  // Return value of 0 signifies error and results in immediate termination
  // of the thread. No other methods are called.

  if(op_arg == 0) op_arg = new Operator::Arg;

  op_arg->fEventor      = this;
  op_arg->fMultix	= bMultix;
  op_arg->fSignalSafe	= bSignalSafe;
  op_arg->fContinuous	= bContinuous;
  op_arg->fUseDynCast	= bUseDynCast;

  return op_arg;
}

void Eventor::PostDance(Operator::Arg* op_arg)
{
  // Eventor::PostDance() is called from Mountain::OperatorBeat() when Eventor
  // throws Operator::OE_Done to request termination of the thread.
  // If Operator::OE_Stop or Operator::OE_Break is thrown, this method
  // is *not* called, but either OnStop() or OnBreak().
}

/**************************************************************************/

void Eventor::PreBeat(Operator::Arg* op_arg) throw(Operator::Exception)
{
  // Eventor::PreBeat() is called from Mountain::OperatorBeat() prior to
  // calling Operate.

  switch(mTimeSource) {
  case TS_System:
    mInternalTime = op_arg->fBeatStart.ToDouble() - mTimeEpoch;
    break;
  case TS_IntStep:
    mInternalTime += mTimeStep;
    break;
  }

  mTimeStack.clear();
  mTimeStack.push_back(mInternalTime);
}

void Eventor::PostBeat(Operator::Arg* op_arg) throw(Operator::Exception)
{
  // Eventor::PostBeat() is called from Mountain::OperatorBeat()
  // after Operate (unless an exception was thrown from Operate's traversal).

  ++mLocBeatsDone;
  bool done = (mBeatsToDo != -1 && mLocBeatsDone >= mBeatsToDo);
  if(mStampInterval == 0 || mLocBeatsDone % mStampInterval == 0 || done) {
    OP_EXE_OR_SP_MIR(this, SetBeatsDone, mLocBeatsDone);
  }
  if(done) {
    throw( Operator::Exception(this, Operator::OE_Done, "reached required beat count") );
  }
}

/**************************************************************************/
// Handlers for Start/Suspend/Exit
/**************************************************************************/

void Eventor::OnStart(Operator::Arg* op_arg)
{
  OP_EXE_OR_SP_MIR(this, SetRunning, true);
  OP_EXE_OR_SP_MIR(this, SetXMultix, op_arg->fMultix);

  // Set epoch on start; for multix moons use the streamed value.
  // ???? Does gettimeofday return the UTC time?
  // If not, must offset the epoch by correct amount on moons..
  if((op_arg->fMultix && IsSunOrFireSpace()) || !op_arg->fMultix) {
    if(mEpochType == ET_DanceStart)
      mTimeEpoch = op_arg->fStart.ToDouble();

    if(!op_arg->fMultix)
      OP_EXE_OR_SP_MIR(this, SetTimeEpoch, mTimeEpoch);
  }

  SetPerforming(true);
}

void Eventor::OnSuspend(Operator::Arg* op_arg)
{
  OP_EXE_OR_SP_MIR(this, SetSuspended, true);
}

void Eventor::OnResume(Operator::Arg* op_arg)
{
  OP_EXE_OR_SP_MIR(this, SetSuspended, false);
}

void Eventor::OnExit(Operator::Arg* op_arg)
{
  if(bSuspended) {
    OP_EXE_OR_SP_MIR(this, SetSuspended, false);
  }
  OP_EXE_OR_SP_MIR(this, SetRunning, false);
  OP_EXE_OR_SP_MIR(this, SetTotalTime, (op_arg->fStop - op_arg->fStart).ToDouble());
  OP_EXE_OR_SP_MIR(this, SetRunTime, op_arg->fBeatSum.ToDouble());
  SetPerforming(false);
}

/**************************************************************************/
// Handlers for different exceptions
/**************************************************************************/

void Eventor::OnContinue(Operator::Arg* op_arg, Operator::Exception& op_exc)
{
  // Called from Mountain::DancerBeat() upon catching "continue" exception.
}

void Eventor::OnWait(Operator::Arg* op_arg, Operator::Exception& op_exc)
{
  // Called from Mountain::DancerBeat() upon catching "wait" exception.
}

void Eventor::OnStop(Operator::Arg* op_arg, Operator::Exception& op_exc)
{
  // Called from Mountain::DancerBeat() upon catching "stop" exception.
}

void Eventor::OnBreak(Operator::Arg* op_arg, Operator::Exception& op_exc)
{
  // Called from Mountain::DancerBeat() upon catching "break" exception.
}

/**************************************************************************/
// User interface for thread control
/**************************************************************************/

void Eventor::Start()
{
  static const Exc_t _eh("Eventor::Start ");

  if(mHost == 0) {
    SetHost(GetQueen()->GetKing()->GetSaturnInfo());
  }

  if(!bMultix && mHost != mSaturn->GetSaturnInfo()) return;

  if(bRunning) {
    ISwarn(_eh + "already running.");
    //return;
  }
  if(bSuspended) {
    ISwarn(_eh + "paused; use Resume to continue.");
    //return;
  }
  mSaturn->GetChaItOss()->Start(this);
}

void Eventor::Stop()
{
  static const Exc_t _eh("Eventor::Stop ");

  if(!bXMultix && mHost != mSaturn->GetSaturnInfo()) return;

  if(!bRunning) {
    ISwarn(_eh + "not running.");
    //return;
  }

  mSaturn->GetChaItOss()->Stop(this);
}

void Eventor::Suspend()
{
  static const Exc_t _eh("Eventor::Suspend ");

  if(!bXMultix && mHost != mSaturn->GetSaturnInfo()) return;

  if(!bRunning) {
    ISwarn(_eh + "not running.");
    //return;
  }

  if(bSuspended) {
    ISwarn(_eh + "already suspended.");
    //return;
  }

  mSaturn->GetChaItOss()->Suspend(this);
}

void Eventor::Resume()
{
  static const Exc_t _eh("Eventor::Resume ");

  if(!bXMultix && mHost != mSaturn->GetSaturnInfo()) return;

  if(!bRunning) {
    ISwarn(_eh + "not running.");
    //return;
  }

  if(!bSuspended) {
    ISwarn(_eh + "not paused.");
    //return;
  }

  mSaturn->GetChaItOss()->Resume(this);
}

void Eventor::Reset()
{
  mTotalTime = mRunTime = 0;
  mBeatsDone = mLocBeatsDone = 0; Stamp(FID());
}

void Eventor::Cancel()
{
  static const Exc_t _eh("Eventor::Cancel ");

  if(!bXMultix && mHost != mSaturn->GetSaturnInfo()) return;

  if(!bRunning) {
    ISwarn(_eh + "not running.");
    //return;
  }

  mSaturn->GetChaItOss()->Cancel(this);
}

/**************************************************************************/

void Eventor::SetHost(SaturnInfo* host)
{
  static const Exc_t _eh("Eventor::SetHost ");

  WriteLock();
  try {
    if(bRunning)
      throw(_eh + "cannot change host while thread is running.");
    set_link_or_die(mHost.ref_link(), host, FID());
  }
  catch(...) {
    WriteUnlock();
    throw;
  }
  WriteUnlock();
}


/**************************************************************************/
// Time service for operators
/**************************************************************************/

Double_t Eventor::GetEventTime()
{
  return mTimeStack.back();
}

void Eventor::PushEventTime(Double_t time)
{
  mTimeStack.push_back(time);
}

void Eventor::PopEventTime()
{
  mTimeStack.pop_back();
}
