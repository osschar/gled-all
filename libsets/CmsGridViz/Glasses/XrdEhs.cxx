// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdEhs.h"
#include "XrdMonSucker.h"
#include "XrdEhs.c7"

#include "TSystem.h"
#include <Glasses/ZHashList.h>
#include <Gled/GTime.h>
#include <ehs/ehs.h>
#include <ehs/ehstypes.h>


#include "XrdFile.h"
#include "XrdUser.h"
#include "XrdServer.h"

#include <iostream>
#include <sstream>
#include <string>

#include <cstdlib>


ClassImp(XrdEhs);

//==============================================================================
// EHS helper
//==============================================================================

namespace
{
  class FormTester : public EHS
  {
  public:
    FormTester (XrdMonSucker* x ) : mRef( x ) { }
    FormTester () : mRef( 0 ) { m_oParams.size(); }

    ResponseCode HandleRequest ( HttpRequest *, HttpResponse * );

    TimeStamp_t    mStamp;
    list<XrdFile*> mList;
    XrdMonSucker*  mRef;
  };

  ResponseCode FormTester::HandleRequest(HttpRequest* request, HttpResponse* response)
  {
    // Creates a page based on user input -- either displays data from
    // form or presents a form for users to submit data.

    enum EMode { kAll, kFileNames, kHelp };

    EMode mode = kAll;

    // std::cerr << request->Uri() << endl;

    if (! request->FormValues().empty()) {
      mode = kHelp;
      if (request->FormValues("mode").m_sBody.length())
      {
        if ( request->FormValues("mode").m_sBody == "all")
          mode = kAll;
        else if ( request->FormValues("mode").m_sBody == "files")
          mode = kFileNames;
        else 
          mode = kHelp;
      }
    }

    ostringstream oss;

    if ( mode == kHelp)
    {
      std::string name = Form("http://%s:%s/?mode=", gSystem->HostName(),  m_oParams["port"].GetCharString());


      oss << "<b>HELP</b>, possible options are: <br><br> " << endl;
      oss << "<a href=\"" << name << "files\">" << name << "files </a> <br>" << endl;
      oss << "<a href=\"" << name << "all\">" << name << "all </a>";
      response->SetBody( oss.str().c_str(), oss.str().length() );
      return HTTPRESPONSECODE_200_OK;
    }


    ZHashList* hl = mRef->GetOpenFiles();
  
    if (mStamp != hl->GetTimeStamp() )
    {
      mList.clear();
      hl->CopyListByGlass<XrdFile>(mList);
      mStamp = hl->GetTimeStamp();
    }

    oss << "<html><body><head><title>Xrd open files ["<< mList.size() << "]</title></head>" << endl;

    oss << "<meta http-equiv=\"refresh\" content=\"180\" />" << endl;

    oss << "<script type=\"text/javascript\" src=\"http://uaf-2.t2.ucsd.edu/~alja/sorttable.js \"> </script>"<< endl;
    oss << ""<< endl;
    oss << "<style type=\"text/css\">"<< endl;
    oss << "th, td {"<< endl;
    oss << "  padding: 3px !important;"<< endl;
    oss << "}"<< endl;
    oss << ""<< endl;
    oss << "table.sortable thead {"<< endl;
    oss << "    background-color:#eee;"<< endl;
    oss << "    color:#666666; "<< endl;
    oss << "    font-weight: bold;"<< endl;
    oss << "    cursor: default; "<< endl;
    oss << "}"<< endl;
    oss << "</style>"<< endl;
    oss << "<br>"<< endl;

    oss << ""<< endl;
    oss << "   <table class=\"sortable\">"<< endl;
    oss << "  <tr>"<< endl;
    oss << "    <th>File</th> " << endl;
    if (mode == kAll) { 
      oss << " <th>OpenTime</th> <th>ServerDomain</th>  <th>ClientDomain</th>  <th>User</th>  <th>Read [MB]</th>  <th>LastUpdateAgo</th>"<< endl;
    }
    oss << "  </tr>"<< endl;


    for (list<XrdFile*>::iterator xfi = mList.begin(); xfi != mList.end(); ++xfi)
    {
      oss << "<tr>"<< endl;
      oss << Form("<td>%s</td>", (*xfi)->GetName()) << endl;
      if (mode == kAll)
      {
        oss << Form("<td>%s</td>", (*xfi)->GetOpenTime().ToDateTimeLocal().Data()) << endl;
        oss << Form("<td>%s</td>", (*xfi)->GetUser()->GetServer()->GetDomain()) << endl;
        oss << Form("<td>%s</td>", (*xfi)->GetUser()->GetFromDomain()) << endl;
        oss << Form("<td>%s</td>", (*xfi)->GetUser()->GetRealName()) << endl;

        oss << Form("<td>%.3f</td>", (*xfi)->GetReadStats().GetSumX()) << endl;
        GTime lastUp = (*xfi)->GetLastMsgTime();
        if (lastUp.IsNever())
        {
          oss << "<td>Never</td>\n";
        }
        else
        {
          int sinceUp =  (int) ((GTime::Now() - lastUp).GetSec());
          int hours = (sinceUp)/3600;
          int min = (sinceUp - hours*3600)/60;
          int sec = sinceUp - hours*3600 - min * 60;
          oss << Form("<td>%02d:%02d:%02d</td>", hours, min, sec) << endl;
        }
      }
      oss << "</tr>" << std::endl;
      
    }    
    oss << "</table>" ;
    oss << "</body></html>";


    response->SetBody( oss.str().c_str(), oss.str().length() );

    return HTTPRESPONSECODE_200_OK;
  }
}


//==============================================================================
// XrdEhs
//==============================================================================

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

void XrdEhs::StartServer()
{
  static const Exc_t _eh("XrdEhs::StartServer ");

  assert_xrdsucker(_eh);

  if (bServerUp)
    throw _eh + "server already running.";

  FormTester srv(*mXrdSucker);
  EHSServerParameters oSP;

  oSP["port"] = mPort; // argv [ 1 ];
  oSP["mode"] = "singlethreaded"; 
  // oSP["mode"] = "threadpool";
  // oSP["threadcount"] = 1; // 1 is default anyway

  // oSP["https"] = 1;
  // oSP["certificate"] = argv [ 2 ];
  // oSP["passphrase"] = argv [ 3 ];
                
  srv.StartServer(oSP);
  
  SetServerUp(true);
  b_stop_server = false;
  while (b_stop_server == false)
  {
    srv.HandleData(1000); // waits for 1 second
  }

  srv.StopServer();
  SetServerUp(false);
}

void XrdEhs::StopServer()
{
  static const Exc_t _eh("XrdEhs::StopServer ");

  if ( ! bServerUp)
    throw _eh + "server not running.";

  b_stop_server = true; 
}
