// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdEhs.h"
#include "XrdMonSucker.h"
#include "XrdEhs.c7"

#include <Glasses/ZHashList.h>
#include <Gled/GThread.h>

#include "XrdFile.h"
#include "XrdUser.h"
#include "XrdServer.h"

#include "TSystem.h"
#include "TServerSocket.h"

#include <iostream>
#include <sstream>
#include <string>

#include <cstdlib>

// XrdEhs

//______________________________________________________________________________
// Status reporting embedded http server.
//
// This used EHS once and supported ?mode=all|files|help but it locked itself
// up regularly and so it got dumped.
//
// TODO:
// - Needs proper thread cancellation.
// - Read the request.
// - Would be nice to return proper header, beyond 200 OK.

ClassImp(XrdEhs);


XrdEhs::~XrdEhs()
{}

void XrdEhs::_init()
{}

XrdEhs::XrdEhs(const Text_t* n, const Text_t* t) :
   ZNameMap(n, t),
   mPort(9999),
   bServerUp(false)
{
  _init();
}

//==============================================================================

void XrdEhs::fill_content(const GTime& req_time, TString& content)
{
  GMutexHolder _lck(mServeMutex);

  if ((req_time - mServeTime) > GTime(1, 0))
  {
    ZHashList* hl = mXrdSucker->GetOpenFiles();
    if (mFileListTS != hl->GetTimeStamp())
    {
      mFileList.clear();
      mFileListTS = hl->CopyListByGlass<XrdFile>(mFileList);
    }

    ostringstream oss;

    oss << "<html><body><head><title>Xrd open files ["<< mFileList.size() << "]</title></head>" << endl;

    oss << "<meta http-equiv=\"refresh\" content=\"180\" />" << endl;

    oss << "<script type=\"text/javascript\" src=\"http://uaf-2.t2.ucsd.edu/~alja/sorttable.js \"> </script>" << endl;
    oss << endl;
    oss << "<style type=\"text/css\">" << endl;
    oss << "th, td {" << endl;
    oss << "  padding: 3px !important;" << endl;
    oss << "}" << endl;
    oss << endl;
    oss << "table.sortable thead {" << endl;
    oss << "    background-color:#eee;" << endl;
    oss << "    color:#666666; " << endl;
    oss << "    font-weight: bold;" << endl;
    oss << "    cursor: default; " << endl;
    oss << "}" << endl;
    oss << "</style>" << endl;
    oss << "<br>" << endl;
    oss << endl;
    oss << "<table class=\"sortable\">"<< endl;
    oss << "<tr>"<< endl;
    oss << "<th>File</th>";
    // if (mode == kAll)
    { 
      oss << " <th>OpenAgo</th> <th>ServerDomain</th> <th>ClientDomain</th> <th>User</th> <th>Read [MB]</th> <th>UpdateAgo</th>";
    }
    oss << endl;
    oss << "</tr>" << endl;

    for (list<XrdFile*>::iterator xfi = mFileList.begin(); xfi != mFileList.end(); ++xfi)
    {
      XrdFile *file = *xfi;
      oss << "<tr>"<< endl;
      oss << Form("<td>%s</td>", file->GetName()) << endl;
      // if (mode == kAll)
      {
        oss << "<td>" << (req_time - file->RefOpenTime()).ToHourMinSec() << "</td>" << endl;
        oss << "<td>" << file->GetUser()->GetServer()->GetDomain() << "</td>" << endl;
        oss << "<td>" << file->GetUser()->GetFromDomain() << "</td>" << endl;
        oss << "<td>" << file->GetUser()->GetRealName() << "</td>" << endl;

        oss << "<td>" << GForm("%.3f", file->GetReadStats().GetSumX()) << "</td>" << endl;
        oss << "<td>" << (req_time - file->RefLastMsgTime()).ToHourMinSec() << "</td>" << endl;
      }
      oss << "</tr>" << endl;
    }    
    oss << "</table>" << endl;
    oss << "</body>"  << endl;
    oss << "</html>"  << endl;

    mServeContent = oss.str();
  }

  content = mServeContent;
}

//==============================================================================

namespace
{
  struct serve_page_arg
  {
    XrdEhs  *xehs;
    TSocket *sock;
    serve_page_arg(XrdEhs* e, TSocket *s) : xehs(e), sock(s) {}
  };

  void* serve_page_tl(serve_page_arg* arg)
  {
    arg->xehs->ServePage(arg->sock);
    delete arg;
    return 0;
  }
}

void XrdEhs::ServePage(TSocket* sock)
{
  GTime now(GTime::I_Now);

  TString content;
  fill_content(now, content);

  // sock->SetOption(kNoBlock, 1);
  // Int_t len = sock->RecvRaw(request, 8192);


  ostringstream hdr;

  hdr << "HTTP/1.1 200 OK" << "\r\n";
  hdr << "Date: " << now.ToWebTimeGMT() << "\r\n";
  hdr << "Connection: close" << "\r\n";
  hdr << "Content-Type: text/html" << "\r\n";
  hdr << "Content-Length: " << content.Length() << "\r\n";
  hdr << "\r\n";

  sock->SendRaw(hdr.str().c_str(), hdr.str().length());
  sock->SendRaw(content.Data(), content.Length());

  GTime::SleepMiliSec(1000);

  sock->Close();
  delete sock;
}

//==============================================================================

void XrdEhs::StartServer()
{
  static const Exc_t _eh("XrdEhs::StartServer ");

  assert_xrdsucker(_eh);

  if (bServerUp)
    throw _eh + "server already running.";

  TServerSocket serv_sock(mPort);
  GSelector     selector;
  selector.fRead.Add(&serv_sock);

  // char request[8192];
  // char erroret[] = "Error processing your request.\n";

  {
    GMutexHolder _lck(mServeMutex);
    mServeTime.SetZero();
    mFileListTS = 0;
  }

  while (! b_stop_server)
  {
    selector.Select();
    TSocket *sock = serv_sock.Accept();

    if (sock == 0)
    {
      ISerr(_eh + "Accept failed, retrying ...");
      continue;
    }

    GThread *thr = new GThread("XrdEhs-PageSender", (GThread_foo) serve_page_tl, new serve_page_arg(this, sock), true);
    thr->SetNice(20);
    thr->Spawn();
  }
}

void XrdEhs::StopServer()
{
  static const Exc_t _eh("XrdEhs::StopServer ");

  if ( ! bServerUp)
    throw _eh + "server not running.";

  b_stop_server = true;
  // should know thread and kill it
}
