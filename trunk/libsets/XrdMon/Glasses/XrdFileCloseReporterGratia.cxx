// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdFileCloseReporterGratia.h"
#include "XrdFileCloseReporterGratia.c7"

#include "XrdFile.h"
#include "XrdUser.h"
#include "XrdServer.h"

#include "Glasses/ZLog.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <cerrno>

// XrdFileCloseReporterGratia

//______________________________________________________________________________
//
//

ClassImp(XrdFileCloseReporterGratia);

//==============================================================================

void XrdFileCloseReporterGratia::_init()
{
  mUdpHost = "localhost";
  mUdpPort = 4242;
}

XrdFileCloseReporterGratia::XrdFileCloseReporterGratia(const Text_t* n, const Text_t* t) :
  XrdFileCloseReporter(n, t),
  mReporterSocket(0)
{
  _init();
}

XrdFileCloseReporterGratia::~XrdFileCloseReporterGratia()
{}

//==============================================================================

void XrdFileCloseReporterGratia::ReportLoopInit()
{
  static const Exc_t _eh("XrdFileCloseReporterGratia::ReportLoopInit ");

  PARENT_GLASS::ReportLoopInit();

  mLastUidBase = mLastUidInner = 0;

  mSAddr = new struct sockaddr;
  {
    struct addrinfo *result;
    struct addrinfo  hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_INET; // AF_UNSPEC; this pulls out IP6 for localhost
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags    = AI_NUMERICSERV;

    int error = getaddrinfo(mUdpHost, TString::Format("%hu", mUdpPort), &hints, &result);
    if (error != 0)
      throw _eh + "getaddrinfo failed: " + gai_strerror(error);

    memcpy(mSAddr, result->ai_addr, sizeof(struct sockaddr));

    freeaddrinfo(result);
  }

  if ((mReporterSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    throw _eh + "socket failed: " + strerror(errno);
}

namespace
{
  Long64_t dtoll (Double_t x) { return static_cast<Long64_t>(x);   }
  Double_t dmtod (Double_t x) { return 1024.0 * 1024.0 * x;        }
  Long64_t dmtoll(Double_t x) { return dtoll(dmtod(x));            }
}

void XrdFileCloseReporterGratia::ReportFileClosed(FileUserServer& fus)
{
  static const Exc_t _eh("XrdFileCloseReporterGratia::ReportFileClosed ");

  XrdFile   *file   = fus.fFile;
  XrdUser   *user   = fus.fUser;
  XrdServer *server = fus.fServer;

  TString msg("#begin\n");

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
      ("unique_id=xrd-%lld\n"
       "file_lfn=%s\nfile_size=%lld\nstart_time=%llu\nend_time=%llu\n"
       "read_bytes=%lld\nread_operations=%llu\nread_min=%lld\nread_max=%lld\nread_average=%f\nread_sigma=%f\n"
       "read_single_bytes=%lld\nread_single_operations=%llu\nread_single_min=%lld\nread_single_max=%lld\nread_single_average=%f\nread_single_sigma=%f\n"
       "read_vector_bytes=%lld\nread_vector_operations=%llu\nread_vector_min=%lld\nread_vector_max=%lld\nread_vector_average=%f\nread_vector_sigma=%f\n"
       "read_vector_count_min=%lld\nread_vector_count_max=%lld\nread_vector_count_average=%f\nread_vector_count_sigma=%f\n"
       "write_bytes=%lld\nwrite_operations=%llu\nwrite_min=%lld\nwrite_max=%lld\nwrite_average=%f\nwrite_sigma=%f\n"
       "read_bytes_at_close=%lld\n"
       "write_bytes_at_close=%lld\n",
       unique_id,
       file->GetName(), dmtoll(file->GetSizeMB()), file->RefOpenTime().GetSec(), file->RefCloseTime().GetSec(),
       dmtoll(RS .GetSumX()), RS .GetN(), dmtoll(RS .GetMin()), dmtoll(RS .GetMax()), dmtod(RS .GetAverage()), dmtod(RS .GetSigma()),
       dmtoll(RSS.GetSumX()), RSS.GetN(), dmtoll(RSS.GetMin()), dmtoll(RSS.GetMax()), dmtod(RSS.GetAverage()), dmtod(RSS.GetSigma()),
       dmtoll(RSV.GetSumX()), RSV.GetN(), dmtoll(RSV.GetMin()), dmtoll(RSV.GetMax()), dmtod(RSV.GetAverage()), dmtod(RSV.GetSigma()),
                                          dtoll(RSVC.GetMin()), dtoll(RSVC.GetMax()), RSVC.GetAverage(),       RSVC.GetSigma(),
       dmtoll(WS .GetSumX()), WS .GetN(), dmtoll(WS .GetMin()), dmtoll(WS .GetMax()), dmtod(WS .GetAverage()), dmtod(WS .GetSigma()),
       dmtoll(file->GetRTotalMB()),
       dmtoll(file->GetWTotalMB()));
    user = file->GetUser();
  }
  {
    GLensReadHolder _ulck(user);
    msg += TString::Format
      ("user_dn=%s\nuser_vo=%s\nuser_role=%s\nuser_fqan=%s\nclient_domain=%s\nclient_host=%s\n"
       "server_username=%s\napp_info=%s\n",
       user->GetDN(), user->GetVO(), user->GetRole(), user->GetGroup(),
       user->GetFromDomain(), user->GetFromHost(), user->GetServerUsername(), user->GetAppInfo());
    server = user->GetServer();
  }
  {
    GLensReadHolder _slck(server);
    msg += TString::Format
      ("server_domain=%s\nserver_host=%s\n",
       server->GetDomain(), server->GetHost());
  }

  msg += "#end\n";

  if (sendto(mReporterSocket, msg.Data(), msg.Length() + 1, 0, mSAddr, sizeof(struct sockaddr)) == -1)
  {
    ZLog::Helper log(*mLog, ZLog::L_Error, _eh);
    log.Form("Error sending report for file='%s'.\n\t%s.",
	     file->GetName(), strerror(errno));
  }
}

void XrdFileCloseReporterGratia::ReportLoopFinalize()
{
  PARENT_GLASS::ReportLoopFinalize();

  close(mReporterSocket);
  mReporterSocket = 0;
}
