// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TRootXTReq.h"

#include <Gled/GCondition.h>
#include <Gled/GThread.h>

#include <TSystem.h>
#include <TTimer.h>

// TRootXTReq

//______________________________________________________________________________
//
//

ClassImp(TRootXTReq);

list<TRootXTReq*>  TRootXTReq::sQueue;
GThread           *TRootXTReq::sRootThread = 0;
GMutex            *TRootXTReq::sQueueMutex = 0;
bool               TRootXTReq::sSheduled   = false;


//==============================================================================

TRootXTReq::TRootXTReq(const char* n) :
  m_return_condition(0),
  mName(n)
{}

TRootXTReq::~TRootXTReq()
{
  delete m_return_condition;
}

//------------------------------------------------------------------------------

void TRootXTReq::post_request()
{
  GMutexHolder _lck(*sQueueMutex);

  sQueue.push_back(this);

  if ( ! sSheduled)
  {
    sSheduled = true;
    sRootThread->Kill(GThread::SigUSR1);
  }
}

void TRootXTReq::ShootRequest()
{
  // Places request into the queue and requests execution in Rint thread.
  // It returns immediately after that, without waiting for execution.
  // The request is deleted after execution.

  if (m_return_condition)
  {
    delete m_return_condition;
    m_return_condition = 0;
  }

  post_request();
}

void TRootXTReq::ShootRequestAndWait()
{
  // Places request into the queue, requests execution in Rint thread and
  // waits for the execution to be completed.
  // The request is not deleted after execution as it might carry return
  // value.
  // The same request can be reused several times.

  if (!m_return_condition)
    m_return_condition = new GCondition;

  m_return_condition->Lock();

  post_request();

  m_return_condition->Wait();
  m_return_condition->Unlock();
}


//==============================================================================

namespace
{
  class XTReqTimer : public TTimer
  {
  public:
    XTReqTimer() : TTimer() {}
    virtual ~XTReqTimer() {}

    void FireAway()
    {
      Reset();
      gSystem->AddTimer(this);
    }

    virtual Bool_t Notify()
    {
      gSystem->RemoveTimer(this);
      TRootXTReq::ProcessQueue();
      return kTRUE;
    }
  };

  XTReqTimer l_xtreq_timer;

  void l_sigusr1_handler(GSignal*)
  {
    ISdebug(1, "Rint thread Usr1 signal handler -- firing timer for XT requests.");
    l_xtreq_timer.FireAway();
  }
};

//------------------------------------------------------------------------------

void TRootXTReq::Bootstrap(GThread* root_thread)
{
  // Should be called from the Rint thread.

  static const Exc_t _eh("TRootXTReq::Bootstrap ");

  if (sRootThread != 0)
    throw _eh + "Already initialized.";

  sRootThread = root_thread;
  sQueueMutex = new GMutex(GMutex::recursive);

  GThread::SetSignalHandler(GThread::SigUSR1, l_sigusr1_handler);
}

void TRootXTReq::Shutdown()
{
  static const Exc_t _eh("TRootXTReq::Shutdown ");

  if (sRootThread == 0)
    throw _eh + "Have not beem initialized.";

  // Should lock and drain queue ... or sth.

  sRootThread = 0;
  GThread::SetSignalHandler(GThread::SigUSR1, 0);
  delete sQueueMutex; sQueueMutex = 0;
}

void TRootXTReq::ProcessQueue()
{
  ISdebug(1, "TRootXTReq::ProcessQueue Timer fired, processing queue.");

  while (true)
  {
    TRootXTReq *req = 0;
    {
      GMutexHolder _lck(*sQueueMutex);

      if ( ! sQueue.empty())
      {
	req = sQueue.front();
	sQueue.pop_front();
      }
      else
      {
	sSheduled = false;
	break;
      }
    }

    req->Act();

    if (req->m_return_condition)
    {
      req->m_return_condition->LockSignal();
    }
    else
    {
      delete req;
    }
  }
}
