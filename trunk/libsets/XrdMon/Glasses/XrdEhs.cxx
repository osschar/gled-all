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
// - Read the request. (Only 'GET' line is read and parsed for path, args.)
// - Would be nice to return proper header, beyond 200 OK.
//   (Somewhat done, but pointless without full request parsing, I reckon.)

ClassImp(XrdEhs);

//==============================================================================

void XrdEhs::_init()
{}

XrdEhs::XrdEhs(const Text_t* n, const Text_t* t) :
   ZNameMap(n, t),
   m_req_line_re("^GET\\s+/?(.*)\\s+HTTP/([\\d\\.]+)$", "o"),
   m_req_re("^([^?]*)(?:\\?(.*))?$", "o"),
   mPort(4242),
   bServerUp(false),
   bParanoia(false)
{
  _init();
}

XrdEhs::~XrdEhs()
{}

//==============================================================================

void XrdEhs::fill_content(const GTime& req_time, TString& content, lStr_t& path, mStr2Str_t& args)
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

    oss << " <th>OpenAgo</th> <th>ServerDomain</th> <th>ClientDomain</th>";
    if (!bParanoia)
      oss << "<th>User</th> ";
    else 
      oss << "<th>UserID</th> ";

    oss << "<th>Read [MB]</th> <th>UpdateAgo</th>";
    oss << endl;
    oss << "</tr>" << endl;

    bool no_same_site = (args["no_same_site"] == "1");
    TPMERegexp short_domain("[^\\.]+\\.[^\\.]+$", "o");

    TPMERegexp srv_re, cli_re, usr_re, fil_re;

    bool f_srv = ( ! args["server_re"].IsNull());
    if (f_srv) srv_re.Reset(args["server_re"], "o");

    bool f_cli = ( ! args["client_re"].IsNull());
    if (f_cli) cli_re.Reset(args["client_re"], "o");

    bool f_usr = ( ! args["user_re"].IsNull());
    if (f_usr) usr_re.Reset(args["user_re"], "o");

    bool f_fil = ( ! args["file_re"].IsNull());
    if (f_fil) fil_re.Reset(args["file_re"], "o");

    TPMERegexp reStoreData("/store/data/(\\w+)/");
    TPMERegexp reStoreMC("/store/mc/(\\w+)/");
    TPMERegexp rePath("/");

    for (list<XrdFile*>::iterator xfi = mFileList.begin(); xfi != mFileList.end(); ++xfi)
    {
      XrdFile *file = *xfi;
      XrdUser *user = file->GetUser();

      if (no_same_site)
      {
        short_domain.Match(user->GetServer()->RefDomain());
        TString srv = short_domain[0];
        short_domain.Match(user->RefFromDomain());
        TString clt = short_domain[0];
        if (srv == clt)
          continue;
      }
      if (f_srv && ! srv_re.Match(user->GetServer()->RefDomain())) continue;
      if (f_cli && ! cli_re.Match(user->RefFromDomain()))          continue;
      if (f_usr && ! usr_re.Match(user->RefRealName()))            continue;
      if (f_fil && ! fil_re.Match(file->RefName()))                continue;

      oss << "<tr>"<< endl; 
        
      if (!bParanoia)
      {
        oss << Form("<td>%s</td>", file->GetName()) << endl;
      }
      else
      {
        if (file->RefName().BeginsWith("/store/user/"))
          oss << "<td>/store/user</td>" << endl;
        else if (rePath.Split(file->RefName()) > 3)
          oss << Form("<td>/%s/%s/%s</td>", rePath[1].Data(), rePath[2].Data(), rePath[3].Data()) <<endl; 
        else
          oss << Form("<td>%s</td>", file->GetName()) << endl;
      }
         
      oss << "<td>" << (req_time - file->RefOpenTime()).ToHourMinSec() << "</td>" << endl;
      oss << "<td>" << user->GetServer()->GetDomain() << "</td>" << endl;
      oss << "<td>" << user->GetFromDomain() << "</td>" << endl;
      if (bParanoia &&  user->RefRealName().Length() ) {
        oss << "<td>" << Form("%X",  user->RefRealName().Hash()) <<  "</td>" << endl;
      }
      else {
        oss << "<td>" << user->GetRealName() << "</td>" << endl;
      }

      oss << "<td>" << GForm("%.3f", file->GetReadStats().GetSumX()) << "</td>" << endl;
      oss << "<td>" << (req_time - file->RefLastMsgTime()).ToHourMinSec() << "</td>" << endl;
         
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
    try
    {
      arg->xehs->ServePage(arg->sock);
    }
    catch (Exc_t& e)
    {
      arg->sock->SendRaw(e.Data(), e.Length());
    }
    arg->sock->Close();
    delete arg->sock;
    delete arg;
    return 0;
  }
}

void XrdEhs::ServePage(TSocket* sock)
{
  static const Exc_t _eh("XrdEhs::ServePage ");

  GTime now(GTime::I_Now);

  const Int_t buf_size = 4096;
  char        buf[buf_size];

  {
    Int_t len = 0;
    while (1)
    {
      Int_t ret = sock->RecvRaw(&buf[len], 1);
      if (ret < 0) {
        throw _eh + GForm("Error sucking %d.", ret);
      }
      if (buf[len] == 10 || buf[len] == 13) {
        buf[len] = 0;
        break;
      }
      if (++len >= buf_size)
        throw _eh + "Request longer than 4k, eat it yourself.";
    }
  }
  TString req_line(buf);
  req_line.ReplaceAll("%20", " ");
  TString path, args;
  {
    GMutexHolder _re_lck(m_re_mutex);
    if (m_req_line_re.Match(req_line) != 3)
      throw _eh + "Strange request line: '" + req_line + "'.";
    Int_t n_req = m_req_re.Match(m_req_line_re[1]);
    if (n_req != 2 && n_req != 3)
      throw _eh + "Strange URL: '" + m_req_line_re[1] + "'.";
    path = m_req_re[1];
    args = m_req_re[2];
  }

  TString xxx = "<p><hr><p>";

  lStr_t path_list;
  if ( ! path.IsNull())
  {
    TPMERegexp sp("/+");
    Int_t np = sp.Split(path);
    xxx += "Request path: '" + path + "'<p>";
    for (Int_t n = 0; n < np; ++n)
    {
      xxx += GForm("%2d '%s'<p>", n + 1, sp[n].Data());
      path_list.push_back(sp[n]);
    }

    xxx += "<p>";
  }

  mStr2Str_t args_map;
  if ( ! args.IsNull())
  {
    xxx += "Request args: '" + args + "'<p>";
    TPMERegexp sa("&+");
    Int_t na = sa.Split(args);
    TPMERegexp sv("=");
    for (Int_t n = 0; n < na; ++n)
    {
      if (sv.Split(sa[n]) != 2)
        throw _eh + "URL parameter error: '" + sa[n] + "'.";
      xxx += GForm("%2d '%s' = '%s'<p>", n + 1, sv[0].Data(), sv[1].Data());
      args_map[sv[0]] = sv[1];
    }
  }

  TString content;
  fill_content(now, content, path_list, args_map);

  content += xxx;

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
}

//==============================================================================

void XrdEhs::StartServer()
{
  static const Exc_t _eh("XrdEhs::StartServer ");

  assert_xrdsucker(_eh);

  {
    GMutexHolder _lck(mServeMutex);

    if (bServerUp)
      throw _eh + "server already running.";

    // char request[8192];
    // char erroret[] = "Error processing your request.\n";

    mServeTime.SetZero();
    mFileListTS = 0;

    bServerUp = true;
    b_stop_server = false;
  }

  TServerSocket serv_sock(mPort);
  GSelector     selector;
  selector.fRead.Add(&serv_sock);

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

  {
    GMutexHolder _lck(mServeMutex);
    bServerUp = false;
  }
}

void XrdEhs::StopServer()
{
  static const Exc_t _eh("XrdEhs::StopServer ");

  {
    GMutexHolder _lck(mServeMutex);

    if ( ! bServerUp)
      throw _eh + "server not running.";

    b_stop_server = true;
    // should know thread and kill it
  }
}
