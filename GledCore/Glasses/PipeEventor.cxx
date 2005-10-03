// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// PipeEventor
//
// Spawns mCommand via gSystem->PipeOpen() in write mode.
// You can send data to be written to the pipe with SendCommand().
// 

#include "PipeEventor.h"
#include <Gled/GThread.h>

#include <TSystem.h>

#include <signal.h>

ClassImp(PipeEventor)

/**************************************************************************/

void PipeEventor::_init()
{
  mPipe = 0;
  mWaitTimeMS = 1000;

  bSignalSafe = true;
}

/**************************************************************************/

void PipeEventor::OnStart(Operator::Arg* op_arg)
{
  Eventor::OnStart(op_arg);

  sigset_t set;

  sigemptyset(&set);
  sigaddset(&set, GThread::SigPIPE);
  pthread_sigmask(SIG_BLOCK, &set, 0);

  mPipe = gSystem->OpenPipe(mCommand.Data(), "w");
  if(!mPipe) {
    ISerr(GForm("PipeEventor::OnStart [%s] OpenPipe(%s) failed",
		GetName(), mCommand.Data()));
    return;
  }
  setlinebuf(mPipe);
}

void PipeEventor::OnExit(Operator::Arg* op_arg)
{
  if(mPipe) {
    gSystem->ClosePipe(mPipe);
    mPipe = 0;
  }
  ClearPendingCommands();

  Eventor::OnExit(op_arg);
}

/**************************************************************************/

void PipeEventor::Operate(Operator::Arg* op_arg) throw(Operator::Exception)
{
  if(!mPipe) {
    throw(Operator::Exception(this, Operator::OE_Break, "pipe closed"));
  }
  
  mSendCond.Lock();
  try {
    feed_commands();
    mSendCond.TimedWaitMS(mWaitTimeMS);
    feed_commands();
  }
  catch(Operator::Exception exc) {
    mSendCond.Unlock();
    throw;
  }
  mSendCond.Unlock();
}

/**************************************************************************/

void PipeEventor::PostCommand(TString& command)
{
  mSendCond.Lock();
  mPending.push_back(command);
  mSendCond.Signal();
  mSendCond.Unlock();
}

void PipeEventor::ClearPendingCommands()
{
  mSendCond.Lock();
  mPending.clear();
  mSendCond.Unlock();
}
/**************************************************************************/
// Protected methods
/**************************************************************************/

void PipeEventor::feed_commands()
{
  // Feeds commands (if any) from mPending to mPipe.
  // Caller must lock the mSendCond.

  if(ferror(mPipe)) {
    throw( Operator::Exception(this, Operator::OE_Break, "pipe closed") );
  }
  
  while(!mPending.empty()) {
    int ret = fprintf(mPipe, "%s\n", mPending.front().Data());
    if(ret < 0 || ferror(mPipe)) {
      throw( Operator::Exception(this, Operator::OE_Break, "write error") );
    }
    mPending.pop_front();
  }
}


/**************************************************************************/

#include "PipeEventor.c7"
