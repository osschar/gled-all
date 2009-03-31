// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// EHSWrap
//
//

#include "EHSWrap.h"
#include "EHSWrap.c7"

#include "ehs_formtest.cpp"

ClassImp(EHSWrap)

/**************************************************************************/

void EHSWrap::_init()
{
  // *** Set all links to 0 ***
  mPort = 9999;
}

/**************************************************************************/

void EHSWrap::StartServer()
{
  const TString _eh("EHSWrap::StartServer ");

  if(bServerUp)
    throw(_eh + "server already running.");

  FormTester srv;
  EHSServerParameters oSP;

  oSP["port"] = mPort; // argv [ 1 ];
  oSP [ "mode" ] = "singlethreaded"; 
  // oSP [ "mode" ] = "threadpool";
  // oSP["threadcount"] = 1; // 1 is default anyway

  // oSP["https"] = 1;
  // oSP["certificate"] = argv [ 2 ];
  // oSP["passphrase"] = argv [ 3 ];
                
  EHS::StartServerResult ssr = srv.StartServer ( oSP );
  if(ssr != EHS::STARTSERVER_SUCCESS)
    throw(_eh + GForm("start server failed status=%d.", ssr));
  
  SetServerUp(true);
  b_stop_server = false;
  while (b_stop_server == false ) {
    srv.HandleData ( 1000 ); // waits for 1 second
  }

  srv.StopServer ( );
  SetServerUp(false);
}

void EHSWrap::StopServer()
{
  const TString _eh("EHSWrap::StopServer ");

  if(! bServerUp)
    throw(_eh + "server not running.");

  b_stop_server = true;
}
/**************************************************************************/
