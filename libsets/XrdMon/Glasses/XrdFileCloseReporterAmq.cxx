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
  mAmqQueue = "/topic/xrdpop.uscms_test_popularity";
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

  TString msg("#begin\n");

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
      ("unique_id=xrd-%lld\n"
       "file_lfn=%s\nfile_size=%lld\nstart_time=%llu\nend_time=%llu\n"
       "read_bytes=%lld\nread_operations=%llu\nread_min=%lld\nread_max=%lld\nread_average=%f\nread_sigma=%f\n"
       "read_single_bytes=%lld\nread_single_operations=%llu\nread_single_min=%lld\nread_single_max=%lld\nread_single_average=%f\nread_single_sigma=%f\n"
       "read_vector_bytes=%lld\nread_vector_operations=%llu\nread_vector_min=%lld\nread_vector_max=%lld\nread_vector_average=%f\nread_vector_sigma=%f\n"
       "read_vector_count_min=%lld\nread_vector_count_max=%lld\nread_vector_count_average=%f\nread_vector_count_sigma=%f\n"
       "write_bytes=%lld\nwrite_operations=%llu\nwrite_min=%lld\nwrite_max=%lld\nwrite_average=%f\nwrite_sigma=%f\n"
       "read_bytes_at_close=%lld\n"
       "write_bytes_at_close=%lld\n",
       0xdead0beef0feed00ll,
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
      ("user_dn=%s\nuser_vo=%s\nuser_role=%s\nuser_fqan=%s\nclient_domain=%s\nclient_host=%s\n"
       "server_username=%s\napp_info=%s\n",
       user->GetDN(), user->GetVO(), user->GetRole(), user->GetGroup(),
       user->GetFromDomain(), user->GetFromHost(), user->GetServerUsername(), user->GetAppInfo());
  }
  {
    GLensReadHolder _slck(server);
    msg += TString::Format
      ("server_domain=%s\nserver_host=%s\n",
       server->GetDomain(), server->GetHost());
  }

  msg += "#end\n";

  // if (sendto(mReporterSocket, msg.Data(), msg.Length() + 1, 0, mSAddr, sizeof(struct sockaddr)) == -1)
  // {
  //   ZLog::Helper log(*mLog, ZLog::L_Error, _eh);
  //   log.Form("Error sending report for file='%s'.\n\t%s.",
  //            file->GetName(), strerror(errno));
  // }
}

void XrdFileCloseReporterAmq::ReportLoopFinalize()
{
  // close(mReporterSocket);
  // mReporterSocket = 0;
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
