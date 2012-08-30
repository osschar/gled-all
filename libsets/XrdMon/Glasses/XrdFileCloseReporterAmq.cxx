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
  mAmqHost  = "gridmsg007.cern.ch";
  mAmqPort  = 6163;
  mAmqUser  = "xrdpop";
  mAmqPswd  = "xyzz";
  mAmqTopic = "xrdpop.uscms_test_popularity";

  mConn = 0;
  mSess = 0;
  mDest = 0;
  mProd = 0;

  mAmqMaxMsgQueueLen   = 1024;
  mAmqReconnectWaitSec = 1;

  // These are local, should be initialized in amq-handler
  mAmqTotalConnectSuccessCount = 0;
  mAmqTotalConnectFailCount = 0;
  mAmqCurrentConnectFailCount = 0;
  bAmqConnected = false;

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
      mLog->Form(ZLog::L_Error, _eh, "Exception callback invoked: '%s'. Reconnection attempt will start now.",
		 e.getStackTraceString().c_str());

  if (mConn)     mConn->close();
  delete mConn;  mConn = 0;

  // mReporterThread->Cancel();
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
    auto_ptr<cms::ConnectionFactory> conn_fac
      (new activemq::core::ActiveMQConnectionFactory(uri.Data(), mAmqUser.Data(), mAmqPswd.Data()));

    mConn = conn_fac->createConnection();
    mConn->setExceptionListener(this);
    mConn->start();
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
    mProd->setDeliveryMode(cms::DeliveryMode::NON_PERSISTENT); // Copied from examples, NFI.
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

}

void* XrdFileCloseReporterAmq::tl_AmqHandler(XrdFileCloseReporterAmq* fcr_amq)
{
  fcr_amq->AmqHandler();

  return 0;
}

void XrdFileCloseReporterAmq::AmqHandler()
{
  // Reset counters;

  amq_connect();



  // Hmmh ... what do we do at cancellation?
  // Klomp the thread, release messages and close_amq from the ReportLoopFinalize()
}


//==============================================================================

void XrdFileCloseReporterAmq::ReportLoopInit()
{
  static const Exc_t _eh("XrdFileCloseReporterAmq::ReportLoopInit ");

  mLastUidBase = mLastUidInner = 0;

  mAmqThread = new GThread("XrdFileCloseReporterAmq-AmqHandler",
                           (GThread_foo) tl_AmqHandler, this,
                           false);
  mAmqThread->SetNice(20);
  mAmqThread->Spawn();

  // The sucker below can fail at any point, just as the send afterwards.
  // Run the connection thingy in a separate thread
  //   a) keep it alive afterwards;
  //   b) run it for every reconnection, as needed.
  // Prefer b ... although it requires more crap. Could just keep it hanging
  // on a condition variable.
  // Hmmh, what if connection times out? Keep the thread going.
  // What do we need for this krappe ...
  // thread foo, foo
  // condvar, thread*
  // separate start/stop amq foos
  // vars to control reconnection time, counters, state bool vars
  // vars to control buffering of messages (later)

}

namespace
{
  Long64_t dtoll (Double_t x) { return static_cast<Long64_t>(x);   }
  Double_t dmtod (Double_t x) { return 1024.0 * 1024.0 * x;        }
  Long64_t dmtoll(Double_t x) { return dtoll(dmtod(x));            }
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

    Long64_t unique_id = 1000ll * file->RefCloseTime().ToMiliSec();
    if (unique_id == mLastUidBase)
    {
      unique_id = mLastUidBase + ++mLastUidInner;
      if (mLastUidInner >= 1000ll)
      {
        ZLog::Helper log(*mLog, ZLog::L_Warning, _eh);
        log.Form("Inner counter for unique-id overflowed for file='%s'.", file->GetName());
        return;
      }
    }
    else
    {
      mLastUidBase  = unique_id;
      mLastUidInner = 0;
    }

    const SRange &RS   = file->RefReadStats();
    const SRange &RSS  = file->RefSingleReadStats();
    const SRange &RSV  = file->RefVecReadStats();
    const SRange &RSVC = file->RefVecReadCntStats();
    const SRange &WS   = file->RefWriteStats();
    msg += TString::Format
      ("'unique_id':'xrd-%lld', "
       "'file_lfn':'%s', 'file_size':'%lld', 'start_time':'%llu', 'end_time':'%llu', "
       "'read_bytes':'%lld', 'read_operations':'%llu', 'read_min':'%lld', 'read_max':'%lld', 'read_average':'%f', 'read_sigma':'%f', "
       "'read_single_bytes':'%lld', 'read_single_operations':'%llu', 'read_single_min':'%lld', 'read_single_max':'%lld', 'read_single_average':'%f', 'read_single_sigma':'%f', "
       "'read_vector_bytes':'%lld', 'read_vector_operations':'%llu', 'read_vector_min':'%lld', 'read_vector_max':'%lld', 'read_vector_average':'%f', 'read_vector_sigma':'%f', "
       "'read_vector_count_min':'%lld', 'read_vector_count_max':'%lld', 'read_vector_count_average':'%f', 'read_vector_count_sigma':'%f', "
       "'write_bytes':'%lld', 'write_operations':'%llu', 'write_min':'%lld', 'write_max':'%lld', 'write_average':'%f', 'write_sigma':'%f', "
       "'read_bytes_at_close':'%lld', "
       "'write_bytes_at_close':'%lld', ",
       unique_id,
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
    msg += TString::Format
      ("'user_dn':'%s', 'user_vo':'%s', 'user_role':'%s', 'user_fqan':'%s', 'client_domain':'%s', 'client_host':'%s', "
       "'server_username':'%s', 'app_info':'%s', ",
       user->GetDN(), user->GetVO(), user->GetRole(), user->GetGroup(),
       user->GetFromDomain(), user->GetFromHost(), user->GetServerUsername(), user->GetAppInfo());
  }
  {
    GLensReadHolder _slck(server);
    msg += TString::Format
      ("'server_domain':'%s', 'server_host':'%s'",
       server->GetDomain(), server->GetHost());
  }

  msg += "}";

  TPMERegexp requote("'", "g");
  requote.Substitute(msg, "\"", kFALSE);

  // This shit can crap out on me, keeping file/user/server reffed for too
  // long. Could actually have the thread that connects managet those guys, I
  // just pass them the strings.
  // This goes to amq-handler now ... here we just lock, push message to queue
  // and signal condition.

  try
  {
    auto_ptr<cms::TextMessage> aqm( mSess->createTextMessage(msg.Data()) );
    mProd->send(aqm.get());
  }
  catch (cms::CMSException& e)
  {
    // Log error, flag & time-stamp that we are not connected, initiate reconnect.
    if (*mLog)
      mLog->Form(ZLog::L_Error, _eh, "Exception during sending of a message: '%s'. Reconnection attempt will start now.",
		 e.getStackTraceString().c_str());

    // What do we need ... shall we run it in a dedicated thread?
    // Hmya, there is no other way, esp. if we want to keep processing messages.
    //   a) ZMIR* S_Reconnect in detached thread?
    //   b) Direct thread creation ... I guess this is best.
    // Must also use non-zero cond-wait time if queue needs to be purged.
    // So that we don't get damn zombies taking over. Well, they don't harm, I think.
  }
}

void XrdFileCloseReporterAmq::ReportLoopFinalize()
{
  static const Exc_t _eh("XrdFileCloseReporterAmq::ReportLoopFinalize ");

  try
  {
    delete mDest;  mDest = 0;
    delete mProd;  mProd = 0;
    if (mSess)     mSess->close();
    if (mConn)     mConn->close();
    delete mSess;  mSess = 0;
    delete mConn;  mConn = 0;
  }
  catch (cms::CMSException& e)
  {
    // Just log it ... we don't really care at this point.
    if (*mLog)
      mLog->Form(ZLog::L_Error, _eh, "", "Exception during ActiveMQ object destruction: '%s'.",
		 e.getStackTraceString().c_str());
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
