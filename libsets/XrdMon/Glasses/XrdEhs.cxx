// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdEhs.h"
#include "XrdMonSucker.h"
#include "XrdEhs.c7"

#include "XrdFile.h"
#include "XrdUser.h"
#include "XrdServer.h"

#include "Glasses/ZHashList.h"
#include "Gled/GThread.h"
#include "Stones/SServerSocket.h"

#include "TSystem.h"

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

    bool fqhn         = (args["fqhn"] == "1");
    bool no_same_site = (args["no_same_site"] == "1");

    int  file_len = 64;
    if ( ! args["file_len"].IsNull()) file_len = TMath::Max(0, args["file_len"].Atoi());

    TPMERegexp short_domain("[^\\.]+\\.[^\\.]+$", "o");

    TPMERegexp srv_re, cli_re, usr_re, fil_re;
    bool       f_srv,  f_cli,  f_usr,  f_fil;

    try
    {
      f_srv = ( ! args["server_re"].IsNull());
      if (f_srv) srv_re.Reset(args["server_re"], "o");

      f_cli = ( ! args["client_re"].IsNull());
      if (f_cli) cli_re.Reset(args["client_re"], "o");

      f_usr = ( ! args["user_re"].IsNull());
      if (f_usr) usr_re.Reset(args["user_re"], "o");

      f_fil = ( ! args["file_re"].IsNull());
      if (f_fil) fil_re.Reset(args["file_re"], "o");
    }
    catch (std::exception& exc)
    {
      content = GForm("Exception caught during parsing of regular expressions:<br>&nbsp;&nbsp;%s\n",
                      exc.what());
      return;
    }

    bool any_fil  = no_same_site || f_srv || f_cli || f_usr || f_fil;
    int  pass_cnt = 0;

    ostringstream oss;

    TPMERegexp rePath("/");
    bool odd = false;
    for (list<XrdFile*>::iterator xfi = mFileList.begin(); xfi != mFileList.end(); ++xfi)
    {
      odd = !odd;
      XrdFile *file = *xfi;
      XrdUser *user = file->GetUser();

      TString server_id = fqhn ? user->GetServer()->GetFqhn() : user->GetServer()->RefDomain();
      TString client_id = fqhn ? user->GetFromFqhn()          : user->RefFromDomain();

      if (no_same_site)
      {
        short_domain.Match(user->GetServer()->RefDomain());
        TString srv = short_domain[0];
        short_domain.Match(user->RefFromDomain());
        TString clt = short_domain[0];
        if (srv == clt)
          continue;
      }
      if (f_srv && ! srv_re.Match(server_id))           continue;
      if (f_cli && ! cli_re.Match(client_id))           continue;
      if (f_usr && ! usr_re.Match(user->RefRealName())) continue;
      if (f_fil && ! fil_re.Match(file->RefName()))     continue;

      ++pass_cnt;

      oss << Form("<tr class='row%d'>", odd ? 1 : 2)<< endl; 

      if (! bParanoia)
      {
        TString fn(file->RefName());
        if (file_len && fn.Length() > file_len) {
          fn.Resize(file_len);
          fn += "...";
        }
        oss << "<td>" << fn << "</td>" << endl;
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

      if (bParanoia && ! user->RefRealName().IsNull())
      {
        oss << "<td>" << Form("%X",  user->RefRealName().Hash()) <<  "</td>" << endl;
      }
      else
      {
        oss << "<td>" << user->GetRealName() << "</td>" << endl;
      }
      oss << "<td>" << server_id << "</td>" << endl;
      oss << "<td>" << client_id << "</td>" << endl;

      GTime    open_t = req_time - file->RefOpenTime();
      GTime    lmsg_t = req_time - file->RefLastMsgTime();

      oss << "<td>" << open_t.ToHourMinSec(true) << "</td>" << endl;
      oss << "<td>" << lmsg_t.ToHourMinSec(true) << "</td>" << endl;

      Double_t siz_mb = file->GetSizeMB();
      Double_t sum_mb = file->GetReadStats().GetSumX();
      Double_t t_rate = (open_t - lmsg_t).ToDouble();  if (t_rate < 0.001) t_rate = 0.001;

      Bool_t f_stream = (sum_mb == 0 && file->GetRTotalMB() > 0);
      if (f_stream)
      {
        sum_mb = file->GetRTotalMB();
      }
      oss << "<td>" << GForm("%.3f", sum_mb) << "</td>" << endl;
      oss << "<td>" << GForm("%.3f", siz_mb ? 100.0*sum_mb/siz_mb : 0.0) << "</td>" << endl;
      oss << "<td>" << GForm("%.3f", sum_mb / t_rate) << "</td>" << endl;
      if (f_stream)
      {
        oss << "<td>n/a</td>" << endl;
      }
      else
      {
        oss << "<td>" << GForm("%.3f", file->GetReadStats().GetAverage()) << "</td>" << endl;
      }

      oss << "</tr>" << endl;
    }
    oss << "</table>" << endl;
    oss << "</body>"  << endl;
    oss << "</html>"  << endl;


    // Generate table header

    ostringstream osh;
    osh << "<html>" << std::endl;
    osh << "<meta http-equiv=\"refresh\" content=\"180\" />" << endl;
    osh << "<head> <meta http-equiv=\"Content-Type\" content=\"text/html; charset=iso-8859-1\"> " << std::endl;
    osh << "<style type=\"text/css\">" << std::endl << std::endl;
     
    osh << "table.demo {" << std::endl;
    osh << "   background-color: #C0C0FF;" << std::endl;
    osh << "   padding: 2px 5px 2px 5px;" << std::endl;
    osh << "}" << std::endl;
    osh << "tr.row1 {" << std::endl;
    osh << "   background-color: #FFFFFF;" << std::endl;
    osh << "}" << std::endl;
    osh << "tr.row2 {" << std::endl;
    osh << "   background-color: #E0E0FF;" << std::endl;
    osh << "}" << std::endl;
    osh << "</style> "<< std::endl;
    osh << "<title>Xrd open files [";
    if (any_fil) osh << pass_cnt << "/";
    osh << mFileList.size() << "]</title> " << std::endl;
    osh << "</head>" << std::endl;
    
    osh << "<script type=\"text/javascript\" src=\"http://uaf-2.t2.ucsd.edu/~alja/gs_sortable.js\"></script>" << std::endl;
    osh << "<script type=\"text/javascript\">" << std::endl;
    osh << "" << std::endl;
   
    osh << "TSort_Data = new Array ('table_xrd_cms_openfiles', 's',  's', 's', 's', 's', 's', 'f', 'f', 'f', 'f');" << std::endl;
    osh << "TSort_Classes = new Array ('row2', 'row1');" << std::endl;
    osh << "TSort_Initial = new Array ('0D');" << std::endl;
    osh << "var TSort_Icons = new Array (' V', ' &#923;');" << std::endl;
    osh << "var TSort_Cookie = 'table_xrd_cms_openfiles';" << std::endl;
    osh << "tsRegister();" << std::endl;
    osh << "</script>" << std::endl;


    osh << "<TABLE id=\"table_xrd_cms_openfiles\" class=\"demo\" width=100%>" << std::endl;
    osh << "<thead>"  << std::endl;

    // header
    osh << "<tr>"<< endl;
    osh << "<th align=\"left\">File</th>";
    osh << "<th align=\"left\">User" << (bParanoia ? " Hash" : "") << "</th>";
    osh << "<th align=\"left\">Server" << (fqhn ? "" : " Domain") << "</th>";
    osh << "<th align=\"left\">Client" << (fqhn ? "" : " Domain") << "</th>";
    osh << "<th align=\"left\">Open Ago</th>";
    osh << "<th align=\"left\">Update Ago</th>";
    osh << "<th align=\"left\">Read [MB]</th>";
    osh << "<th align=\"left\">Read [%]</th>";
    osh << "<th align=\"left\">Rate [MB/s]</th>";
    osh << "<th align=\"left\">Avg Read [MB]</th>";
    osh << endl;
    osh << "</tr>" << endl;

    osh << "</thead>" << std::endl;

    content  = osh.str();
    content += oss.str();
  }
}

//==============================================================================

namespace
{
  struct serve_page_arg
  {
    XrdEhs  *xehs;
    SSocket *sock;
    serve_page_arg(XrdEhs* e, SSocket *s) : xehs(e), sock(s) {}
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

void XrdEhs::ServePage(SSocket* sock)
{
  static const Exc_t _eh("XrdEhs::ServePage ");

  GTime now = GTime::ApproximateTime();

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
  if (! path.IsNull())
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
  if (! args.IsNull())
  {
    xxx += "Request args: '" + args + "'<p>";
    TPMERegexp sa("&+");
    Int_t na = sa.Split(args);
    TPMERegexp sv("=");
    for (Int_t n = 0; n < na; ++n)
    {
      Int_t nsa = sv.Split(sa[n]);
      if (nsa < 1 || nsa > 2)
        throw _eh + "URL parameter error: '" + sa[n] + "'.";
      if (nsa == 2) {
	xxx += GForm("%2d. '%s' = '%s'<p>", n + 1, sv[0].Data(), sv[1].Data());
	args_map[sv[0]] = sv[1];
      } else {
	xxx += GForm("%2d. '%s' = '1'<p>", n + 1, sv[0].Data());
	args_map[sv[0]] = "1";
      }
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

    mServeTime.SetZero();
    mFileListTS = 0;

    bServerUp = true;
    b_stop_server = false;
  }

  SServerSocket serv_sock(mPort, true);
  if (! serv_sock.IsValid())
  {
    throw _eh + "Creation of server socket failed.";
  }

  GSelector     selector;
  selector.fRead.Add(&serv_sock);

  while (! b_stop_server)
  {
    selector.Select();
    SSocket *sock = serv_sock.Accept();

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
