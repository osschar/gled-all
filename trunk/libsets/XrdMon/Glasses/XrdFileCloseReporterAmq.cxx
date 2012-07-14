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

  mConnFac = 0;
  mConn = 0;
  mSess = 0;
  mDest = 0;
  mProd = 0;
}

XrdFileCloseReporterAmq::XrdFileCloseReporterAmq(const Text_t* n, const Text_t* t) :
  XrdFileCloseReporter(n, t)
{
  _init();
}

XrdFileCloseReporterAmq::~XrdFileCloseReporterAmq()
{}

//==============================================================================

void XrdFileCloseReporterAmq::ReportLoopInit()
{
  static const Exc_t _eh("XrdFileCloseReporterAmq::ReportLoopInit ");

  TString uri;
  uri.Form("failover://(tcp://%s:%hu?wireFormat=stomp)", mAmqHost.Data(), mAmqPort);
  mConnFac = new activemq::core::ActiveMQConnectionFactory(uri.Data(), mAmqUser.Data(), mAmqPswd.Data());

  try
  {
    mConn = mConnFac->createConnection();
    mConn->start();
  }
  catch (cms::CMSException& e)
  {
    throw _eh + "Exception during connection creation: " + e.getStackTraceString();
  }

  mSess = mConn->createSession(); // Default is AUTO_ACKNOWLEDGE
  mDest = mSess->createTopic(mAmqTopic.Data());
  mProd = mSess->createProducer(mDest);
  mProd->setDeliveryMode(cms::DeliveryMode::NON_PERSISTENT); // Copied from examples, NFI.
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

  XrdFile   *file   = fus.fFile;
  XrdUser   *user   = fus.fUser;
  XrdServer *server = fus.fServer;

  TString msg("{");

  {
    GLensReadHolder _flck(file);

    // Long64_t unique_id = 1000ll * file->RefCloseTime().ToMiliSec();
    // if (unique_id == mLastUidBase)
    // {
    //   unique_id = mLastUidBase + ++mLastUidInner;
    //   if (mLastUidInner >= 1000ll)
    //   {
    //     ZLog::Helper log(*mLog, ZLog::L_Warning, _eh);
    //     log.Form("Inner counter for unique-id overflowed for file='%s'.", file->GetName());
    //     return;
    //   }
    // }
    // else
    // {
    //   mLastUidBase  = unique_id;
    //   mLastUidInner = 0;
    // }

    const SRange &RS   = file->RefReadStats();
    const SRange &RSS  = file->RefSingleReadStats();
    const SRange &RSV  = file->RefVecReadStats();
    const SRange &RSVC = file->RefVecReadCntStats();
    const SRange &WS   = file->RefWriteStats();
    msg += TString::Format
      ("'file_lfn':'%s', 'file_size':'%lld', 'start_time':'%llu', 'end_time':'%llu', "
       "'read_bytes':'%lld', 'read_operations':'%llu', 'read_min':'%lld', 'read_max':'%lld', 'read_average':'%f', 'read_sigma':'%f', "
       "'read_single_bytes':'%lld', 'read_single_operations':'%llu', 'read_single_min':'%lld', 'read_single_max':'%lld', 'read_single_average':'%f', 'read_single_sigma':'%f', "
       "'read_vector_bytes':'%lld', 'read_vector_operations':'%llu', 'read_vector_min':'%lld', 'read_vector_max':'%lld', 'read_vector_average':'%f', 'read_vector_sigma':'%f', "
       "'read_vector_count_min':'%lld', 'read_vector_count_max':'%lld', 'read_vector_count_average':'%f', 'read_vector_count_sigma':'%f', "
       "'write_bytes':'%lld', 'write_operations':'%llu', 'write_min':'%lld', 'write_max':'%lld', 'write_average':'%f', 'write_sigma':'%f', "
       "'read_bytes_at_close':'%lld', "
       "'write_bytes_at_close':'%lld', ",
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

  cms::TextMessage* aqm = mSess->createTextMessage(msg.Data());
  mProd->send(aqm);
  delete aqm;
}

void XrdFileCloseReporterAmq::ReportLoopFinalize()
{
  delete mConnFac; mConnFac = 0;
  delete mConn;    mConn = 0;
  delete mSess;    mSess = 0;
  delete mDest;    mDest = 0;
  delete mProd;    mProd = 0;
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
