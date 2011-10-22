// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdEHS.h"
#include "XrdEHS.c7"

#include <Glasses/ZHashList.h>
#include <Gled/GTime.h>
#include <ehs/ehs.h>


#include "XrdMonSucker.h"
#include "XrdFile.h"
#include "XrdUser.h"

#include <iostream>
#include <sstream>
#include <string>

#include <cstdlib>


//==============================================================================
//==============================================================================
//==============================================================================

class  FormTester : public EHS
{
public:
   FormTester (XrdMonSucker* x) : mRef( x ) { }

   ResponseCode HandleRequest ( HttpRequest *, HttpResponse * );

   XrdMonSucker* mRef;
};


// creates a page based on user input -- either displays data from
// form or presents a form for users to submit data.
ResponseCode FormTester::HandleRequest ( HttpRequest * request, HttpResponse * response )
{
   enum EMode { kAll, kFileNames, kHelp };

   EMode mode = kAll;

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
      oss << "ERROR, possible options are: <br><br> http://localhost:4242/?mode=files <br> http://localhost:4242/?mode=all"; 
      response->SetBody( oss.str().c_str(), oss.str().length() );
      return HTTPRESPONSECODE_200_OK;
   }


   oss << "<html><body><head><title>StringList</title></head>" << endl;

   oss << "<meta http-equiv=\"refresh\" content=\"60\" />" << endl;

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
      oss << "<th>FromDomain</th>  <th>User</th>  <th>openTime</th> <th>userDomain</th>"<< endl;
   }
   oss << "  </tr>"<< endl;

   
   ZHashList* hl = mRef->GetOpenFiles();
  
   list<XrdFile*> open_files;
   hl->CopyListByGlass<XrdFile>(open_files);
   for (list<XrdFile*>::iterator xfi = open_files.begin(); xfi != open_files.end(); ++xfi)
   {
      oss << "<tr>"<< endl;
      oss << Form("<td>%s</td>", (*xfi)->GetName()) << endl;
      if (mode == kAll)
      {
         oss << Form("<td>%s</td>", (*xfi)->GetUser()->GetFromDomain()) << endl;
         oss << Form("<td>%s</td>", (*xfi)->GetUser()->GetName()) << endl;
         oss << Form("<td>%s</td>", (*xfi)->GetOpenTime().ToDateTimeLocal().Data()) << endl;
         oss << Form("<td>%s</td>", (*xfi)->GetUser()->GetDN()) << endl;
      }
      oss << "</tr>" << std::endl;
      
   }    
   oss << "</table>" ;
   oss << "</body></html>";


   response->SetBody( oss.str().c_str(), oss.str().length() );


   return HTTPRESPONSECODE_200_OK;

}

//==============================================================================
//==============================================================================
//==============================================================================

ClassImp(XrdEHS);


XrdEHS::~XrdEHS()
{}

void XrdEHS::_init()
{}

XrdEHS::XrdEHS(XrdMonSucker* x) :
   ZNameMap("EHS", "EHS test"),
   mXrdReporter(x)
{
  _init();
  mPort = 9999;
}

//==============================================================================
void XrdEHS::StartServer()
{
  const TString _eh("XrdEHS::StartServer ");

  if (bServerUp)
    throw _eh + "server already running.";

  FormTester srv(mXrdReporter);
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
  while (b_stop_server == false )
  {
    srv.HandleData(1000); // waits for 1 second
  }

  srv.StopServer();
  SetServerUp(false);
}

void XrdEHS::StopServer()
{
  const TString _eh("XrdEHS::StopServer ");

  if ( ! bServerUp)
    throw _eh + "server not running.";

    b_stop_server = true; 
}
