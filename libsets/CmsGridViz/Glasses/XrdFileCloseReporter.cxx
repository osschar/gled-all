// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdFileCloseReporter.h"
#include "XrdFileCloseReporter.c7"

#include "XrdFile.h"
#include "XrdUser.h"
#include "XrdServer.h"

#include <Gled/GThread.h>

#include <cerrno>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// XrdFileCloseReporter

//______________________________________________________________________________
//
//

ClassImp(XrdFileCloseReporter);

//==============================================================================

void XrdFileCloseReporter::_init()
{
  mUdpHost = "desire.physics.ucsd.edu";
  mUdpPort = 7632;
}

XrdFileCloseReporter::XrdFileCloseReporter(const Text_t* n, const Text_t* t) :
  ZGlass(n, t),
  mReporterThread(0),
  mReporterSocket(0)
{
  _init();
}

XrdFileCloseReporter::~XrdFileCloseReporter()
{}

//==============================================================================

void XrdFileCloseReporter::FileClosed(XrdFile* file)
{
  // put into queue and signal condition

  GMutexHolder _lck(mReporterCond);
  mReporterQueue.push_back(file);
  mReporterCond.Signal();
}

//==============================================================================

namespace
{
  Long64_t dtoll (Double_t x) { return static_cast<Long64_t>(x);   }
  Long64_t dmtoll(Double_t x) { return 1024ll * 1024ll * dtoll(x); }
  Double_t dmtod (Double_t x) { return 1024.0 * 1024.0 * x;        }
}

void* XrdFileCloseReporter::tl_ReportLoop(XrdFileCloseReporter* r)
{
  r->ReportLoop();
  r->mReporterThread = 0;
  return 0;
}

void XrdFileCloseReporter::ReportLoop()
{
  static const Exc_t _eh("XrdFileCloseReporter::ReportLoop ");

  struct sockaddr rAddr;
  {
    struct addrinfo *result;
    struct addrinfo  hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;
    hints.ai_flags    = AI_NUMERICSERV;

    int error = getaddrinfo(mUdpHost, TString::Format("%hu", mUdpPort), &hints, &result);
    if (error != 0)
      throw _eh + "getaddrinfo failed: " + gai_strerror(error);

    memcpy(&rAddr, result->ai_addr, sizeof(struct sockaddr));

    freeaddrinfo(result);
  }

  if ((mReporterSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    throw _eh + "socket failed: " + strerror(errno);

  while (true)
  {
    XrdFile   *file   = 0;
    XrdUser   *user   = 0;
    XrdServer *server = 0;
    {
      GMutexHolder _lck(mReporterCond);
      if (mReporterQueue.empty())
        mReporterCond.Wait();
      file = mReporterQueue.front();
      mReporterQueue.pop_front();
    }

    TString msg("#begin\n");

    {
      GLensReadHolder _flck(file);
      const SRange &RS = file->RefReadStats();
      const SRange &WS = file->RefWriteStats();
      msg += TString::Format
        ("file_lfn=%s\nstart_time=%llu\nend_time=%llu\n"
         "read_bytes=%lld\nread_operations=%llu\nread_min=%lld\nread_max=%lld\nread_average=%f\nread_sigma=%f\n"
         "write_bytes=%lld\nwrite_operations=%llu\nwrite_min=%lld\nwrite_max=%lld\nwrite_average=%f\nwrite_sigma=%f\n"
         "read_bytes_at_close=%lld\n"
         "write_bytes_at_close=%lld\n",
         file->GetName(), file->RefOpenTime().GetSec(), file->RefCloseTime().GetSec(),
         dmtoll(RS.GetSumX()), RS.GetN(), dmtoll(RS.GetMin()), dmtoll(RS.GetMax()), dmtod(RS.GetAverage()), dmtod(RS.GetSigma()),
         dmtoll(WS.GetSumX()), WS.GetN(), dmtoll(WS.GetMin()), dmtoll(WS.GetMax()), dmtod(WS.GetAverage()), dmtod(WS.GetSigma()),
         dmtoll(file->GetRTotalMB()),
         dmtoll(file->GetWTotalMB()));
      user = file->GetUser();
    }
    {
      GLensReadHolder _ulck(user);
      msg += TString::Format
        ("user_dn=%s\nuser_vo=%s\nuser_role=%s\nclient_domain=%s\nclient_host=%s\n",
         user->GetDN(), user->GetVO(), user->GetRole(), user->GetFromDomain(), user->GetFromHost());
      server = user->GetServer();
    }
    {
      GLensReadHolder _slck(server);
      msg += TString::Format
        ("server_domain=%s\nserver_host=%s\n",
         server->GetDomain(), server->GetHost());
    }

    msg += "#end\n";

    if (sendto(mReporterSocket, msg.Data(), msg.Length() + 1, 0, &rAddr, sizeof(rAddr)) == -1)
      printf("%sError sending report for file='%s'.\n", _eh.Data(), file->GetName());
  }
}


//==============================================================================

void XrdFileCloseReporter::StartReporter()
{
  static const Exc_t _eh("XrdFileCloseReporter::StartReporter ");

  {
    GLensReadHolder _lck(this);
    if (mReporterThread)
      throw _eh + "already running.";

    mReporterThread = new GThread("XrdFileCloseReporter-Reporter",
                                  (GThread_foo) tl_ReportLoop, this,
                                  false);
  }
  mReporterThread->SetNice(20);
  mReporterThread->Spawn();
}

void XrdFileCloseReporter::StopReporter()
{
  static const Exc_t _eh("XrdFileCloseReporter::StopReporter ");

  GThread *thr = 0;
  {
    GLensReadHolder _lck(this);
    if ( ! GThread::IsValidPtr(mReporterThread))
      throw _eh + "not running.";
    thr = mReporterThread;
    GThread::InvalidatePtr(mReporterThread);
  }
  thr->Cancel();
  thr->Join();
  close(mReporterSocket);
  {
    GLensReadHolder _lck(this);
    mReporterQueue.clear();
    mReporterSocket = 0;
    mReporterThread = 0;
  }
}
