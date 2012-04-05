// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SServerSocket.h"

#include "TROOT.h"
#include "TSystem.h"

// SServerSocket

//______________________________________________________________________________
//
// Almost a clone of TServerSocket, differences being:
// - it instantiates and returns a SSocket;
// - authentication a la TServerSocket is not supported.


ClassImp(SServerSocket);

//==============================================================================

SServerSocket::SServerSocket(const char *service, Bool_t reuse, Int_t backlog,
                             Int_t tcpwindowsize)
{
  // Create a server socket object for a named service. Set reuse to true
  // to force reuse of the server socket (i.e. do not wait for the time
  // out to pass). Using backlog one can set the desirable queue length
  // for pending connections.
  // Use tcpwindowsize to specify the size of the receive buffer, it has
  // to be specified here to make sure the window scale option is set (for
  // tcpwindowsize > 65KB and for platforms supporting window scaling).
  // Use IsValid() to check the validity of the
  // server socket. In case server socket is not valid use GetErrorCode()
  // to obtain the specific error value. These values are:
  //  0 = no error (socket is valid)
  // -1 = low level socket() call failed
  // -2 = low level bind() call failed
  // -3 = low level listen() call failed
  // Every valid server socket is added to the TROOT sockets list which
  // will make sure that any open sockets are properly closed on
  // program termination.

  R__ASSERT(gROOT);
  R__ASSERT(gSystem);

  SetName("ServerSocket");

  fSecContext = 0;

  // If this is a local path, try announcing a UNIX socket service
  ResetBit(TSocket::kIsUnix);
  if (service && (!gSystem->AccessPathName(service) ||
#ifndef WIN32
		  service[0] == '/')) {
#else
    service[0] == '/' || (service[1] == ':' && service[2] == '/'))) {
#endif
      SetBit(TSocket::kIsUnix);
      fService = "unix:";
      fService += service;
      fSocket = gSystem->AnnounceUnixService(service, backlog);
      if (fSocket >= 0) {
         R__LOCKGUARD2(gROOTMutex);
         gROOT->GetListOfSockets()->Add(this);
      }
   } else {
      // TCP / UDP socket
      fService = service;
      int port = gSystem->GetServiceByName(service);
      if (port != -1) {
         fSocket = gSystem->AnnounceTcpService(port, reuse, backlog, tcpwindowsize);
         if (fSocket >= 0) {
            R__LOCKGUARD2(gROOTMutex);
            gROOT->GetListOfSockets()->Add(this);
         }
      } else {
         fSocket = -1;
      }
   }
}

//______________________________________________________________________________
SServerSocket::SServerSocket(Int_t port, Bool_t reuse, Int_t backlog,
                             Int_t tcpwindowsize)
{
   // Create a server socket object on a specified port. Set reuse to true
   // to force reuse of the server socket (i.e. do not wait for the time
   // out to pass). Using backlog one can set the desirable queue length
   // for pending connections. If port is 0 a port scan will be done to
   // find a free port. This option is mutual exlusive with the reuse option.
   // Use tcpwindowsize to specify the size of the receive buffer, it has
   // to be specified here to make sure the window scale option is set (for
   // tcpwindowsize > 65KB and for platforms supporting window scaling).
   // Use IsValid() to check the validity of the
   // server socket. In case server socket is not valid use GetErrorCode()
   // to obtain the specific error value. These values are:
   //  0 = no error (socket is valid)
   // -1 = low level socket() call failed
   // -2 = low level bind() call failed
   // -3 = low level listen() call failed
   // Every valid server socket is added to the TROOT sockets list which
   // will make sure that any open sockets are properly closed on
   // program termination.

   R__ASSERT(gROOT);
   R__ASSERT(gSystem);

   SetName("ServerSocket");

   fSecContext = 0;
   fService = gSystem->GetServiceByPort(port);
   SetTitle(fService);

   fSocket = gSystem->AnnounceTcpService(port, reuse, backlog, tcpwindowsize);
   if (fSocket >= 0) {
      R__LOCKGUARD2(gROOTMutex);
      gROOT->GetListOfSockets()->Add(this);
   }
}

//==============================================================================

SSocket *SServerSocket::Accept()
{
  if (fSocket == -1) { return 0; }

  SSocket *socket = new SSocket;

  Int_t soc = gSystem->AcceptConnection(fSocket);
  if (soc == -1) { delete socket; return 0; }
  if (soc == -2) { delete socket; return (SSocket*) -1; }

  socket->fSocket  = soc;
  socket->fSecContext = 0;
  socket->fService = fService;
  if (!TestBit(TSocket::kIsUnix))
    socket->fAddress = gSystem->GetPeerName(socket->fSocket);
  if (socket->fSocket >= 0) {
    R__LOCKGUARD2(gROOTMutex);
    gROOT->GetListOfSockets()->Add(socket);
  }

  return socket;
}

TInetAddress SServerSocket::GetLocalInetAddress()
{
   // Return internet address of host to which the server socket is bound,
   // i.e. the local host. In case of error TInetAddress::IsValid() returns
   // kFALSE.

   if (fSocket != -1) {
      if (fAddress.GetPort() == -1)
         fAddress = gSystem->GetSockName(fSocket);
      return fAddress;
   }
   return TInetAddress();
}

Int_t SServerSocket::GetLocalPort()
{
   // Get port # to which server socket is bound. In case of error returns -1.

   if (fSocket != -1) {
      if (fAddress.GetPort() == -1)
         fAddress = GetLocalInetAddress();
      return fAddress.GetPort();
   }
   return -1;
}
