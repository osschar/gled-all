// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdFileCloseReporterAmq.h"
#include "XrdFileCloseReporterAmq.c7"

#include "XrdFile.h"
#include "XrdUser.h"
#include "XrdServer.h"

#include "Glasses/ZLog.h"
#include "Gled/GThread.h"

#include "TPRegexp.h"

#include <activemq/library/ActiveMQCPP.h>
#include <activemq/core/ActiveMQConnectionFactory.h>
#include <cms/Connection.h>
#include <cms/Session.h>
#include <cms/TextMessage.h>

#include <cerrno>

// XrdFileCloseReporterAmq

//______________________________________________________________________________
//
//

ClassImp(XrdFileCloseReporterAmq);

//==============================================================================

void XrdFileCloseReporterAmq::_init()
{
  mAmqHost  = "dashb-mb.cern.ch";
  mAmqPort  = 6163;
  mAmqUser  = "xrdpop";
  mAmqPswd  = "xyzz";
  mAmqTopic = "xrdpop.uscms_test_popularity";
  bLeakAmqObjs = false;
  bPersistent  = false;

  mConnFac = 0;
  mConn = 0;
  mSess = 0;
  mDest = 0;
  mProd = 0;
  bConnClosed = true;

  mAmqMaxMsgQueueLen      = 10000;
  mAmqReconnectWaitSec    = 1;
  mAmqReconnectWaitSecMax = 300;
  mAmqAutoReconnectSec    = 0;

  // Some amq-counters state vars get initalized in AmqHandler().

  mAmqThread = 0;
}

XrdFileCloseReporterAmq::XrdFileCloseReporterAmq(const Text_t* n, const Text_t* t) :
  XrdFileCloseReporter(n, t)
{
  _init();
}

XrdFileCloseReporterAmq::~XrdFileCloseReporterAmq()
{}

//==============================================================================

void XrdFileCloseReporterAmq::onException(const cms::CMSException &e)
{
  static const Exc_t _eh("XrdFileCloseReporterAmq::onException ");

  if (*mLog)
      mLog->Form(ZLog::L_Error, _eh, "Exception callback invoked:\n    %s",
		 e.getStackTraceString().c_str());

  if (mConn)
  {
    mConn->close();
    bConnClosed = true;
  }
}

void XrdFileCloseReporterAmq::amq_connect()
{
  static const Exc_t _eh("XrdFileCloseReporterAmq::amq_connect ");

  TString uri;
  // Failover and Stomp don't splice ... or, they splice too well, making as
  // many threads as ulimit lets them on first error ... thrashing the machine.
  // uri.Form("failover://(tcp://%s:%hu?wireFormat=stomp)", mAmqHost.Data(), mAmqPort);
  uri.Form("tcp://%s:%hu?wireFormat=stomp", mAmqHost.Data(), mAmqPort);

  try
  {
    mConnFac = new activemq::core::ActiveMQConnectionFactory(uri.Data(), mAmqUser.Data(), mAmqPswd.Data());

    mConn = mConnFac->createConnection();
    mConn->setExceptionListener(this);
    mConn->start();
    bConnClosed = false;
  }
  catch (cms::CMSException& e)
  {
    throw _eh + "Exception during connection creation: " + e.getStackTraceString();
  }

  try
  {
    mSess = mConn->createSession(); // Default is AUTO_ACKNOWLEDGE
    mDest = mSess->createTopic(mAmqTopic.Data());
    mProd = mSess->createProducer(mDest);
    mProd->setDeliveryMode(bPersistent ? cms::DeliveryMode::PERSISTENT : cms::DeliveryMode::NON_PERSISTENT);
  }
  catch (cms::InvalidDestinationException& e)
  {
    throw _eh + "Invalid destination exception during producer creation: " + e.getStackTraceString();
  }
  catch (cms::CMSException& e)
  {
    throw _eh + "Exception during session, topic or message producer initialization: " + e.getStackTraceString();
  }
}

void XrdFileCloseReporterAmq::amq_disconnect()
{
  static const Exc_t _eh("XrdFileCloseReporterAmq::amq_disconnect ");

  try
  {
    if (mSess)
    {
      mSess->close();
    }
    if (mConn && ! bConnClosed)
    {
      mConn->close();
      bConnClosed = true;
    }
    if ( ! bLeakAmqObjs)
    {
      delete mProd; delete mDest; delete mSess; delete mConn; delete mConnFac;
    }
    mProd = 0; mDest = 0; mSess = 0; mConn = 0; mConnFac = 0;
  }
  catch (cms::CMSException& e)
  {
    // Just log it ... we don't really care at this point.
    if (*mLog)
      mLog->Form(ZLog::L_Error, _eh, "Exception during ActiveMQ object destruction:\n    %s",
		 e.getStackTraceString().c_str());
  }
}

void* XrdFileCloseReporterAmq::tl_AmqHandler(XrdFileCloseReporterAmq* fcr_amq)
{
  GThread::Self()->CleanupPush((GThread_cu_foo) cu_AmqHandler, fcr_amq);

  fcr_amq->AmqHandler();

  return 0;
}

void XrdFileCloseReporterAmq::cu_AmqHandler(XrdFileCloseReporterAmq* fcr_amq)
{
  fcr_amq->amq_disconnect();

  {
    GLensReadHolder _lck(fcr_amq);
    fcr_amq->bAmqConnected = false;
    fcr_amq->Stamp(FID());
  }
}

void XrdFileCloseReporterAmq::AmqHandler()
{
  static const Exc_t _eh("XrdFileCloseReporterAmq::AmqHandler ");

  // Reset counters
  {
    GLensReadHolder _lck(this);
    mAmqTotalConnectSuccessCount = 0;
    mAmqTotalConnectFailCount    = 0;
    mAmqTotalAutoReconnectCount  = 0;
    mAmqCurrentConnectFailCount  = 0;
    mAmqSendMessageFailCount     = 0;
    bAmqConnected                = false;
    Stamp(FID());
  }

  Int_t sleep_seconds = 0;

entry_point:

  if (sleep_seconds > 0)
  {
    GTime::SleepMiliSec(1000 * sleep_seconds);
  }

  try
  {
    amq_connect();
  }
  catch (Exc_t exc)
  {
    // Cleanup ... however far we've got.
    amq_disconnect();

    {
      GLensReadHolder _lck(this); 
      ++mAmqTotalConnectFailCount;
      ++mAmqCurrentConnectFailCount;
      Stamp(FID());
    }

    // Decide how long to sleep
    sleep_seconds = (sleep_seconds == 0) ?
      mAmqReconnectWaitSec :
      TMath::Min(2*sleep_seconds, mAmqReconnectWaitSecMax);

    if (*mLog)
      mLog->Form(ZLog::L_Error, _eh, "Exception during connect:\n    %s"
		 "  Reconnection attempt scheduled in %d seconds.",
		 exc.Data(), sleep_seconds);

    goto entry_point;
  }

  {
    GLensReadHolder _lck(this);
    ++mAmqTotalConnectSuccessCount;
    bAmqConnected = true;
    Stamp(FID());
  }

  GTime auto_reconnect_time = GTime::ApproximateFuture(mAmqAutoReconnectSec);

  while (true)
  {
    // wait on amqcond, send messages, handle exceptions
    TString msg;
    {
      GMutexHolder _qlck(mAmqCond);
      if (mAmqMsgQueue.empty())
      {
        if (mAmqAutoReconnectSec)
        {
          if (mAmqCond.TimedWaitUntil(auto_reconnect_time) == 1)
          {
            break;
          }
        }
        else
        {
          mAmqCond.Wait();
        }
      }
      msg = mAmqMsgQueue.front();
      mAmqMsgQueue.pop_front();
    }

    try
    {
      auto_ptr<cms::TextMessage> aqm( mSess->createTextMessage(msg.Data()) );
      mProd->send(aqm.get());
    }
    catch (cms::CMSException& e)
    {
      amq_disconnect();

      {
	GLensReadHolder _lck(this);
	++mAmqSendMessageFailCount;
	mAmqCurrentConnectFailCount = 0;
	bAmqConnected = false;
	Stamp(FID());
      }

      // Log error, flag & time-stamp that we are not connected, initiate reconnect.
      if (*mLog)
	mLog->Form(ZLog::L_Error, _eh, "Exception during sending of a message:\n    %s"
		   "  Reconnection attempt will start now.",
		   e.getStackTraceString().c_str());

      sleep_seconds = 0;

      goto entry_point;
    }
  }

  // The fact that we're here means we're supposed to do automatic
  // reconnection.
  {
    amq_disconnect();

    {
      GLensReadHolder _lck(this);
      ++mAmqTotalAutoReconnectCount;
      mAmqCurrentConnectFailCount = 0;
      bAmqConnected = false;
      Stamp(FID());
    }

    // Log reconnect request.
    if (*mLog)
      mLog->Form(ZLog::L_Message, _eh, "Disconnect for automatic periodic reconnect initiated (delta_t=%ds).", mAmqAutoReconnectSec);

    sleep_seconds = 0;

    goto entry_point;
  }
}


//==============================================================================

void XrdFileCloseReporterAmq::ReportLoopInit()
{
  static const Exc_t _eh("XrdFileCloseReporterAmq::ReportLoopInit ");

  mAmqThread = new GThread("XrdFileCloseReporterAmq-AmqHandler",
                           (GThread_foo) tl_AmqHandler, this);
  mAmqThread->SetNice(20);
  mAmqThread->Spawn();
}

namespace
{
  Long64_t dtoll (Double_t x) { return static_cast<Long64_t>(x); }
  Double_t dmtod (Double_t x) { return 1024.0 * 1024.0 * x;      }
  Long64_t dmtoll(Double_t x) { return dtoll(dmtod(x));          }
}

void XrdFileCloseReporterAmq::ReportFileClosed(FileUserServer& fus)
{
  static const Exc_t _eh("XrdFileCloseReporterAmq::ReportFileClosed ");

  // Lock amq, check if queue len at max ... if yes, just return.

  XrdFile   *file   = fus.fFile;
  XrdUser   *user   = fus.fUser;
  XrdServer *server = fus.fServer;

  TString msg("{");

  {
    GLensReadHolder _flck(file);

    const SRange &RS   = file->RefReadStats();
    const SRange &RSS  = file->RefSingleReadStats();
    const SRange &RSV  = file->RefVecReadStats();
    const SRange &RSVC = file->RefVecReadCntStats();
    const SRange &WS   = file->RefWriteStats();
    msg += TString::Format
      ("'unique_id':'%s-%llx', "
       "'file_lfn':'%s', 'file_size':'%lld', 'start_time':'%llu', 'end_time':'%llu', "
       "'read_bytes':'%lld', 'read_operations':'%llu', 'read_min':'%lld', 'read_max':'%lld', 'read_average':'%f', 'read_sigma':'%f', "
       "'read_single_bytes':'%lld', 'read_single_operations':'%llu', 'read_single_min':'%lld', 'read_single_max':'%lld', 'read_single_average':'%f', 'read_single_sigma':'%f', "
       "'read_vector_bytes':'%lld', 'read_vector_operations':'%llu', 'read_vector_min':'%lld', 'read_vector_max':'%lld', 'read_vector_average':'%f', 'read_vector_sigma':'%f', "
       "'read_vector_count_min':'%lld', 'read_vector_count_max':'%lld', 'read_vector_count_average':'%f', 'read_vector_count_sigma':'%f', "
       "'write_bytes':'%lld', 'write_operations':'%llu', 'write_min':'%lld', 'write_max':'%lld', 'write_average':'%f', 'write_sigma':'%f', "
       "'read_bytes_at_close':'%lld', "
       "'write_bytes_at_close':'%lld', ",
       mUuid.Data(), mNProcessed,
       file->GetName(), dmtoll(file->GetSizeMB()), file->RefOpenTime().GetSec(), file->RefCloseTime().GetSec(),
       dmtoll(RS .GetSumX()), RS .GetN(), dmtoll(RS .GetMin()), dmtoll(RS .GetMax()), dmtod(RS .GetAverage()), dmtod(RS .GetSigma()),
       dmtoll(RSS.GetSumX()), RSS.GetN(), dmtoll(RSS.GetMin()), dmtoll(RSS.GetMax()), dmtod(RSS.GetAverage()), dmtod(RSS.GetSigma()),
       dmtoll(RSV.GetSumX()), RSV.GetN(), dmtoll(RSV.GetMin()), dmtoll(RSV.GetMax()), dmtod(RSV.GetAverage()), dmtod(RSV.GetSigma()),
                                          dtoll(RSVC.GetMin()), dtoll(RSVC.GetMax()), RSVC.GetAverage(),       RSVC.GetSigma(),
       dmtoll(WS .GetSumX()), WS .GetN(), dmtoll(WS .GetMin()), dmtoll(WS .GetMax()), dmtod(WS .GetAverage()), dmtod(WS .GetSigma()),
       dmtoll(file->GetRTotalMB()),
       dmtoll(file->GetWTotalMB()));
  }
  {
    GLensReadHolder _ulck(user);

    TString dn = user->StrDN(); dn.ReplaceAll("'", "\\'");

    msg += TString::Format
      ("'user_dn':'%s', 'user_vo':'%s', 'user_role':'%s', 'user_fqan':'%s', 'client_domain':'%s', 'client_host':'%s', "
       "'server_username':'%s', 'user_protocol':'%s', 'app_info':'%s', ",
       dn.Data(), user->GetVO(), user->GetRole(), user->GetGroup(),
       user->GetFromDomain(), user->GetFromHost(), user->GetServerUsername(), user->GetProtocol(), user->GetAppInfo());
  }
  {
    GLensReadHolder _slck(server);
    msg += TString::Format
      ("'server_domain':'%s', 'server_host':'%s', 'server_site':'%s'",
       server->GetDomain(), server->GetHost(), server->GetSite());
  }

  msg += "}";

  TPMERegexp requote("'", "g");
  requote.Substitute(msg, "\"", kFALSE);

  // Pass the message on to AmqThread ...
  {
    GMutexHolder _qlck(mAmqCond);
    mAmqMsgQueue.push_back(msg);
    if (mAmqMsgQueue.size() > mAmqMaxMsgQueueLen)
    {
      mAmqMsgQueue.pop_front();
    }
    mAmqCond.Signal();
  }
}

void XrdFileCloseReporterAmq::ReportLoopFinalize()
{
  static const Exc_t _eh("XrdFileCloseReporterAmq::ReportLoopFinalize ");

  mAmqThread->Cancel();
  mAmqThread->Join();
  delete mAmqThread;
  mAmqThread = 0;

  {
    GMutexHolder _qlck(mAmqCond);
    mAmqMsgQueue.clear();
  }
}


//==============================================================================
// XrdMon libset user-init
//==============================================================================

// Temporary location, as AMQ is the only thing that needs global
// initialization and XrdFileCloseReporterAmq is the only thing that uses AMQ.
//
// This should go elsewhere, eventually.

void libXrdMon_GLED_user_init()
{
  activemq::library::ActiveMQCPP::initializeLibrary();
}

void* XrdMon_GLED_user_init = (void*) libXrdMon_GLED_user_init;

void libXrdMon_GLED_user_shutdown()
{
  activemq::library::ActiveMQCPP::shutdownLibrary();
}

void* XrdMon_GLED_user_shutdown = (void*) libXrdMon_GLED_user_shutdown;
