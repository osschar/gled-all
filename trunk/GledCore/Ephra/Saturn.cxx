// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.


//______________________________________________________________________
// Saturn
//
// Server and client. In fact Sun, Moon and Fire.
// Shines, reflects and glows.
//

#include "Saturn.h"

#include <Gled/GledNS.h>
#include <Ephra/Forest.h>
#include <Ephra/Mountain.h>
#include <Glasses/ZGod.h>
#include <Glasses/ZKing.h>
#include <Glasses/ZSunQueen.h>
#include <Glasses/ZQueen.h>
#include <Glasses/ZMirFilter.h>
#include <Gled/GThread.h>
#include <Stones/ZComet.h>
#include <Gled/GledMirDefs.h>

// Services
#include <Stones/ZHistoManager.h>

#include <TROOT.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TSystem.h>


#include <errno.h>
#include <signal.h>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Definitely need Queen loading implemented in Saturn
// GCondition mQueenLoadingCond
// lID_t mAwaited Queens (or hash ... or both ... or sth) (need list<GCondition*>)
// void LoadQueen(ZQueen*) or LoadQueen(ID_t) { checks deps ... }
// void SuspendUntilQueenReady(ZQueen*)
// void SuspendUntilNoQueensPending()
//
// where do requests come from?
// This is ... again ... the tricky part.
// Now been mis-using the fire-king

ClassImp(Saturn)

/**************************************************************************/
// Thread invocators
/**************************************************************************/

namespace {
  GCondition	_server_startup_cond;

  void sh_SaturnFdSucker(int sig) {
    //ISmess(GForm("Saturn/sh_SaturnFdSucker called with signal %d", sig));
  }
}

void* Saturn::tl_SaturnFdSucker(Saturn *s)
{
  // The thread foo for doing select on all sockets (master, moons & eyes).
  // Could (in principle) have several threads.

  GThread::SetCancelState(GThread::CS_Enable);
  GThread::SetCancelType(GThread::CT_Deferred);

  { // Signals init;
    // return from select on USR2 to allow additions of new socket fd's
    sigset_t set;

    sigemptyset(&set);
    sigaddset(&set, GThread::SigUSR2);
    pthread_sigmask(SIG_UNBLOCK, &set, 0);

    struct sigaction sac;
    sac.sa_handler = sh_SaturnFdSucker;
    sigemptyset(&sac.sa_mask);
    sac.sa_flags = 0;
    sigaction(SIGUSR2, &sac, 0);
  }

  GThread::setup_tsd(s->GetSaturnInfo());

  _server_startup_cond.Lock();
  _server_startup_cond.Signal();
  _server_startup_cond.Unlock();
  // Perhaps should install some exit-foo ... to close sockets ...
  while(1) {
    int ret = s->SockSuck();
    if(ret != 0) {
      ISmess(GForm("Saturn::mServerThread %s", strerror(errno)));
    }
    GThread::TestCancel();
  }
  return 0;
}

void* Saturn::tl_SaturnAcceptor(new_connection_ti* ncti)
{
  // The thread foo for accepting a new connection.
  // Accept threads are always created in a detached state.

  GThread::setup_tsd(ncti->sat->GetSaturnInfo());
  ncti->sat->AcceptWrapper(ncti->sock);
  delete ncti;
  delete GThread::get_self();
  GThread::cleanup_tsd();
  GThread::Exit();
  return 0;
}

void* Saturn::tl_MIR_Router(mir_router_ti* arg)
{
  // Thread foo for routing of a single MIR.
  // If the thread is detached, this method also deletes the GThread.

  GThread::setup_tsd(arg->mir->Caller);
  arg->sat->RouteMIR(*arg->mir);

  if(arg->delete_mir)
    delete arg->mir;
  delete arg;
  GThread* self = GThread::get_self();
  if(self->GetDetached()) {
    delete self;
    GThread::cleanup_tsd();
  }
  GThread::Exit();
  return 0;
}

void* Saturn::tl_MIR_Shooter(Saturn* s)
{
  // Thread foo for independent routing of MIRs.
  // Mostly they come from the Saturn code and eventor threads.
  // Continuously calls Saturn->shoot_mir_from_queue().
  // Could have any number of such threads.

  GThread::SetCancelState(GThread::CS_Enable);
  GThread::SetCancelType(GThread::CT_Deferred);

  GThread::setup_tsd(s->GetSaturnInfo());

  while(1) {
    try {
      s->shoot_mir_from_queue();
    }
    catch(string exc) {
      ISmess(GForm("Saturn::mMIRShootingThread caugt exception: %s", exc.c_str()));
    }
    GThread::TestCancel();
  }
  return 0;
}

/**************************************************************************/
// Static members
/**************************************************************************/

const Int_t Saturn::s_Gled_Protocol_Version = 1;

/**************************************************************************/
// Con/De/Structor
/**************************************************************************/

Saturn::Saturn() :
  mIDLock(GMutex::recursive),
  mEyeLock(GMutex::recursive),
  mMoonLock(GMutex::recursive),
  mMasterLock(GMutex::recursive),
  mRulingLock(GMutex::recursive),
  mSelector(GMutex::recursive),
  mSunInfo(0), mSaturnInfo(0),
  mQueenLoadNum(0), mQueenLoadCnd(GMutex::recursive),
  mBeamReqHandleMutex(GMutex::recursive), mLastBeamReqHandle(0),
  mMIRShootingCnd()
{
  bAllowMoons = false;

  mVer = 0; mChaItOss = 0;

  pZHistoManager = 0;

  // GOD
  mGod = new ZGod("God", "I do nothing ... I only am.");
  Enlight(mGod, MAX_ID);

  // mVer = new Forest(this, "Ver", "Forest of Life");
  // mVer->Init();
  
  mChaItOss = new Mountain(this);
}

Saturn::~Saturn() {
  delete mChaItOss;
  delete mVer;
  delete mServerSocket; // Close is called by dtor
}

/**************************************************************************/

string Saturn::HandleClientSideSaturnHandshake(TSocket*& socket)
{
  // Handles handshake with a Saturn on a newly opened socket.
  // Returns greeting string.

  TInetAddress ia = socket->GetInetAddress();
  if(!socket->IsValid()) {
    delete socket; socket = 0;
    throw(string(GForm("opening socket to %s:%d failed.", ia.GetHostName(), ia.GetPort())));
  }

  // Receive greeting
  int ml; char buf[256];
  ml = socket->RecvRaw(buf, 255, kDontBlock);
  if(ml <= 0) {
    delete socket; socket = 0;
    throw(string(GForm("handshake with %s:%d failed: len=%d.", ia.GetHostName(), ia.GetPort(), ml)));
  }
  buf[ml] = 0;
  return string(buf);
}

TMessage* Saturn::HandleClientSideMeeConnection(TSocket* socket, ZMirEmittingEntity* mee)
{
  // Sends request for instantiation of mee via socket.
  // If authentication is required it is handled via
  // ZSunQueen::HandleClientSideAuthentication(). The required identity
  // is deduced from the TString ZMirEmittingEntity::mLogin.

  static const string _eh(""); // caller should prefix the exception

  { // Send the desired Identity & Mee
    TMessage creq(GledNS::MT_MEE_Connect);
    GledNS::StreamGlass(creq, mee);
    socket->Send(creq);
  }
  TMessage* m = 0;
  Int_t     l;
 grant_auth_loop:
  l = socket->Recv(m);
  if(l <= 0) {
    throw(_eh + "connection broken");
  }
  if(m->What() == GledNS::MT_MEE_ConnectionGranted) {
    return m;
  }
  else if(m->What() == GledNS::MT_MEE_AuthRequested) {
    TString host; Int_t port; UInt_t conn_id;
    host.Streamer(*m); *m >> port >> conn_id;
    TSocket* auth_sock = new TSocket(host.Data(), port);
    try {
      Saturn::HandleClientSideSaturnHandshake(auth_sock);
      ZSunQueen::HandleClientSideAuthentication(auth_sock, conn_id, mee->mLogin);
    }
    catch(string exc) {
      delete auth_sock; delete m;
      throw(_eh + "auth failed: " + exc);
    }
    delete auth_sock; delete m;
    goto grant_auth_loop;
  }
  else if(m->What() == GledNS::MT_MEE_ConnectionDenied) {
    TString s; *m >> s;  delete m;
    throw(_eh + "connection denied: " + s.Data());
  }
  else {
    throw(_eh + "unknown response");
  }
}

/**************************************************************************/

void Saturn::Create(SaturnInfo* si)
{
  // Spawns SunAbsolute and starts internal Saturn threads.

  mSunInfo = mSaturnInfo = si;
  mSaturnInfo->hSocket = 0; // No masters above me
  bSunAbsolute = true;

  mSaturnInfo->mKingID = 1;
  fix_fire_king_id(mSaturnInfo);
  create_kings("SunKing", "FireKing");
  mKing->SetTitle("Ruler of what is");
  mFireKing->SetTitle("Ruler of what isn't");

  mSunKing = mKing;

  mSunQueen = new ZSunQueen(666, "SunQueen", "Princess of ChaItOss");
  mSunKing->Enthrone(mSunQueen);
  mSunQueen->CheckIn(mSaturnInfo);
  mSunQueen->mSunInfo = mSaturnInfo;
  mSaturnInfo->create_lists();

  if(start_mir_shooter() || start_server()) {
    exit(1);
  }

  if(GThread::get_self() == 0)
    GThread::wrap_and_register_self(mSaturnInfo);

  try {
    mSunQueen->attach_primary_identity(mSaturnInfo);
  }
  catch(string exc) {
    cerr <<"Saturn::Create exception: "<< exc <<endl;
    exit(1);
  }
}

SaturnInfo* Saturn::Connect(SaturnInfo* si)
{
  // Connects to a higher Saturn, as specified by si->MasterName and
  // si->MasterPort. Does the following:
  // 1) handles handshake & protocol info exchange
  // 2) queries first free id to establish beginning of local sun-space
  // 3) authenticates to sun.absolute and receives higher kings, sun.queen
  //    and the authorized SaturnInfo structure
  // 4) starts MirShooting thread and server thread
  // 5) issues requests for mirroring of mandatory queens
  // The calling thread is given the identity of the return SaturnInfo, which
  // is also assigned to mSaturnInfo member.

  static string _eh("Saturn::Connect ");

  bSunAbsolute = false;

  TSocket* sock = new TSocket(si->GetMasterName(), si->GetMasterPort());
  try {
    string greeting = Saturn::HandleClientSideSaturnHandshake(sock);
    ISmess(_eh + greeting);
  }
  catch(string exc) {
    throw(_eh + exc);
  }

  { // Exchange protocol info
    TMessage m(GledNS::MT_GledProtocol);
    m << s_Gled_Protocol_Version;
    sock->Send(m);
    TMessage* r;
    int rl = sock->Recv(r);
    if(rl <= 0) {
      delete sock;
      throw(_eh + GForm("protocol exchange failed; len=%d",rl));
    }

    if(r->What() == GledNS::MT_GledProtocol) {
      Int_t sproto; *r >> sproto;
      ISmess(GForm("Saturn::Connect server protocol=%d", sproto));
    } else if(r->What() == GledNS::MT_ProtocolMismatch) {
      Int_t sproto; *r >> sproto;
      delete sock; delete r;
      throw(_eh + GForm("protocol mismatch: server=%d, client=%d",
			sproto, s_Gled_Protocol_Version));
    } else {
      Int_t mt = r->What();
      delete sock; delete r;
      throw(_eh + GForm("unknown message type %d", mt));
    }
  }

  { // QueryFFId
    TMessage m(GledNS::MT_QueryFFID);
    sock->Send(m);
    TMessage* r;
    int rl = sock->Recv(r);
    if(rl <= 0) {
      delete sock;
      throw(_eh + "handshake failed (3)");
    }

    if(r->What() == GledNS::MT_QueryFFID) {
      ID_t ffid; *r >> ffid;
      ISmess(GForm("Saturn::Connect got first free id=%d", ffid));

      si->mKingID = ffid;
      if(si->mKingID == 0) throw(_eh + "failed FFID query");
      fix_fire_king_id(si);

    } else {
      Int_t mt = r->What();
      delete sock; delete r;
      throw(_eh + GForm("unknown message type %d", mt));
    }
  }

  {
    TMessage* m;
    try {
      m = HandleClientSideMeeConnection(sock, si);
    }
    catch(string exc) {
      delete sock;
      throw(_eh + exc);
    }
    arrival_of_kings(m);
    delete m;
  }

  create_kings("King of the Moon", "FireKing");
  mKing->SetTitle("Ruler of what partially is");
  mFireKing->SetTitle("Ruler of what isn't");

  mSaturnInfo->hSocket = 0;
  mSaturnInfo->hRoute = 0;
  mSaturnInfo->GetMaster()->hSocket = sock;
  mSaturnInfo->GetMaster()->hRoute  = mSaturnInfo->GetMaster();
  mSelector.fRead[sock->GetDescriptor()] = (void*)sock;
  //sock->SetOption(kNoBlock, 1);

  if(start_mir_shooter() || start_server()) {
    exit(1);
  }

  if(GThread::get_self() == 0)
    GThread::wrap_and_register_self(mSaturnInfo);

  // Issue requests for mandatory queens.
  lpZGlass_t kings; mGod->Copy(kings); kings.pop_back(); kings.pop_back();
  for(lpZGlass_i i=kings.begin(); i!=kings.end(); ++i) {
    ZKing* k = dynamic_cast<ZKing*>(*i);
    lpZGlass_t queens; k->Copy(queens);
    for(lpZGlass_i j=queens.begin(); j!=queens.end(); ++j) {
      ZQueen* q = dynamic_cast<ZQueen*>(*j);
      q->mKing = k;
      if(q->GetMandatory() && ! q->GetRuling()) {
	mFireKing->ReflectQueen(q);
      }
    }
  }
  

  return mSaturnInfo;
}

void Saturn::AllowMoons()
{
  // On start-up the moons are not allowed to connect (only Eyes).
  // AllowMoons() allows moons connections from now on.

  bAllowMoons = true;
}

void Saturn::Shutdown()
{
  // Performs shutdown of connected objects and threads.

  // first should dump moons, eyez ...

  mChaItOss->Shutdown();

  ISmess("Saturn::Shutdown stopping server");
  stop_mir_shooter();
  stop_server();

  // Services shut down
  if(pZHistoManager) {
    pZHistoManager->Write();
    pZHistoManager->Close();
    delete pZHistoManager; pZHistoManager = 0;
  }

}

/**************************************************************************/
// Enlight/Endark of lenses
/**************************************************************************/

void Saturn::Enlight(ZGlass* glass, ID_t id) throw(string)
{
  // Inserts glass into hash with key id.
  // Sets Saturn related glass members and calls ZGlass::AdEnlightenment().

  if(glass == 0)
    throw(string("Saturn::Enlight glass=0; stalling ..."));

  if(id == 0)
    throw(string("Saturn::Enlight id=0; stalling ..."));


  mIDLock.Lock();
  hID2pZGlass_i i = mIDHash.find(id);
  if(i != mIDHash.end()) {
    mIDLock.Unlock();
    throw(string(GForm("Saturn::Enlight id=%u already used by '%s' (new is '%s'; stalling...",
		      id, i->second->GetName(), glass->GetName())));
  }
  mIDHash[id] = glass;
  mIDLock.Unlock();

  // Infrom the new lens
  glass->mSaturnID = id;
  glass->mSaturn = this;
  ISdebug(2,GForm("Saturn::Enlight for %s, id=%u", glass->GetName(), id));
  // Now let the newglass call its initialization foos
  glass->AdEnlightenment();
}

void Saturn::Reflect(ZGlass* glass) throw(string)
{
  // Enlights glass with a key taken from glass->mSaturnID.
  // Used for ZComet unpacking on Moons (hence Reflect).

  if(glass->mSaturnID == 0) throw (string("Saturn::Reflect called w/ id 0"));
  Enlight(glass, glass->mSaturnID);
}

void Saturn::Endark(ZGlass* glass) throw(string)
{
  if(glass==0)
    throw(string("Saturn::Enlight glass=0; stalling ..."));

  // First hack self
  mIDLock.Lock();
  hID2pZGlass_i i;
  if((i=mIDHash.find(glass->mSaturnID)) == mIDHash.end() || i->second != glass) {
    mIDLock.Unlock();
    throw(string("Saturn::Endark id/glass mismatch; stalling..."));
  }
  mIDHash.erase(i);
  mIDLock.Unlock();
  glass->AdEndarkenment();
}

/**************************************************************************/
//
/**************************************************************************/

// Freezing not really needed anymore. Now solved via RulingLock and
// per-queen ExecLock.

Int_t Saturn::Freeze()
{
  mChaItOss->SuspendAll();
  mMoonLock.Lock();
  return 0;
}

Int_t Saturn::UnFreeze()
{
  mMoonLock.Unlock();
  mChaItOss->ResumeAll();
  return 0;
}

/**************************************************************************/
// Broadcasting to Eyes
/**************************************************************************/

void Saturn::Shine(Ray &r)
{
  if(mEyes.empty()) return;

  TMessage msg(GledNS::MT_Ray);
  r.Streamer(msg);
  ISdebug(9, GForm("Saturn::Shine Notifying %d eye(s)", mEyes.size()));
  Int_t len = msg.Length() - 4;
  mEyeLock.Lock();
  lpEyeInfo_i i = mEyes.begin();
  while(i != mEyes.end()) {
    Int_t ret = (*i)->hSocket->Send(msg);
    if(ret != len) {
      ISerr(GForm("Saturn::Shine sent too little: Eye=%s, len=%3d, ret=%3d\n",
		  (*i)->GetName(), len, ret));
    }
    lpEyeInfo_i j = i++;
    if(ret < 0) {
      wipe_eye(*j, true);
    }
  }
  mEyeLock.Unlock();
}

void Saturn::SingleRay(EyeInfo* eye, Ray& ray)
{
  TMessage msg(GledNS::MT_Ray);
  if(eye->hSocket == 0) {
    ISerr(GForm("Saturn::SingleRay got request for non-local eye %s",
		eye->GetName()));
  }
  ray.Streamer(msg);
  mEyeLock.Lock();
  eye->hSocket->Send(msg);
  mEyeLock.Unlock();
}

/**************************************************************************/

Bool_t Saturn::IsMoon(SaturnInfo* si)
{
  mMoonLock.Lock();
  lpSaturnInfo_i l = find(mMoons.begin(), mMoons.end(), si);
  Bool_t ret = (l != mMoons.end());
  mMoonLock.Unlock();
  return ret;
}

/**************************************************************************/
// Saturn services
/**************************************************************************/

ZHistoManager* Saturn::GetZHistoManager()
{
  if(!pZHistoManager) {
    GledNS::PushFD();
    pZHistoManager = new ZHistoManager(GForm("%s.root", mSaturnInfo->GetName()), "RECREATE");
    GledNS::PopFD();
  }
  return pZHistoManager;
}

/**************************************************************************/
// Net Stuff
/**************************************************************************/

Int_t Saturn::SockSuck()
{
  mSelector.fTimeOut = 10;
  Int_t s_stat = mSelector.Select();

  ISdebug(9, GForm("Saturn::SockSuck got %d", s_stat));
  if(s_stat<0) return s_stat;
  if(s_stat==0) return 0;

  for(mFdUD_i i=mSelector.fReadOut.begin(); i!=mSelector.fReadOut.end(); i++) {
    TSocket* s = (TSocket*) i->second;
    if(s==mServerSocket) {
      TSocket* newsocket = mServerSocket->Accept();
      new_connection_ti* ncti = new new_connection_ti(this, newsocket);
      GThread* at = new GThread((GThread_foo)tl_SaturnAcceptor, ncti, true);
      at->Spawn();
      continue;
    }
    try {
      Manage(s);
    }
    catch(string exc) {
      ISerr(GForm("Saturn::SockSuck caught exception: %s", exc.c_str()));
    }
  }

  // !! upon accept should interrupt the select and add new descriptor.
  // threads highly magickal wrt signals.
  // now have to wait till timeout.

  return 0;
}

SaturnInfo* Saturn::FindRouteTo(SaturnInfo* target)
{
  // Finds SaturnInfo that is on-route to target.
  // Checks target's masters and if self is found, sends to
  // the previous master.
  // Otherwise it must be sent up.

  list<SaturnInfo*> t_ml;
  SaturnInfo* p = target;
  do {
    t_ml.push_back(p);
    if(p == mSaturnInfo) {
      return (*(--(--t_ml.end())));
    }
    p = p->GetMaster();
  } while(p);
  return mSaturnInfo->GetMaster();
}

/**************************************************************************/

void Saturn::AcceptWrapper(TSocket* newsocket)
{
  // Exception catching wrapper for Accept().

  static string _eh("Saturn::AcceptWrapper ");

  if(!newsocket) {
    ISerr(_eh + "Accept socket failed");
    return;
  }
  ISdebug(0, GForm("%sConnection from: %s", _eh.c_str(),
		   newsocket->GetInetAddress().GetHostName()));

  try {
    Accept(newsocket);
  }
  catch(string exc) {
    // Perhaps should report to someone
    ISerr(_eh + " exception: " + exc.c_str());
  }
}

void Saturn::Accept(TSocket* newsocket) throw(string)
{
  // Accepts connection from an Eye/Moon or handles FirstFreeID query.
  // Locks/Suspends all executions upon
  // adding fd to Selector and sending scene to the Moon.
  // Runs in a detached thread (called from tl_SaturnAcceptor) ... 
  // perhaps should timeout.
  // Handles one connection at a time.

  static string _eh("Saturn::Accept ");

  // Initial handshake
  char msgbuf[256];
  int  len = snprintf(msgbuf, 256,
		      "This is Saturn \"%s\", Gled version %s (%s). Hello ...",
		      mSaturnInfo->GetName(), GLED_VERSION_STRING, GLED_BUILD_DATE_STRING);
  if(len > 255) len = 255;
  newsocket->SendRaw(msgbuf, len);

  GSelector sel;
  sel.fRead[newsocket->GetDescriptor()] = (void*)newsocket;
  sel.fTimeOut = 20; // close connection after 20s of inactivity

  bool loop_done  = false;
  bool loop_error = false;
  string loop_error_msg;

  while(!loop_done) {

    if(loop_error) {
      delete newsocket;
      throw(_eh + loop_error_msg);
    }

    int ret = sel.Select();

    if(ret <= 0) {
      // do clean-up
      // make descriptive message
      string err_msg = (ret == 0) ? "connection timeout" : "select error";

      delete newsocket;
      throw(_eh + err_msg);
    }

    TMessage* msg;
    int len = newsocket->Recv(msg);
    if(len <= 0) {
      delete newsocket;
      if(len == 0) {
	ISdebug(0, "Saturn::Accept other side closed connection");
	return;
      } else {
	throw(_eh + GForm("error receiving message (len=%d)", len));
      }
    }

    switch (msg->What()) {

    case GledNS::MT_GledProtocol: {
      Int_t client_proto;
      *msg >> client_proto;
      bool compatible_p = (client_proto == s_Gled_Protocol_Version);
      TMessage reply(compatible_p ? GledNS::MT_GledProtocol :
		     GledNS::MT_ProtocolMismatch);
      reply << s_Gled_Protocol_Version;
      newsocket->Send(reply);
      if(!compatible_p) {
	loop_error = true;
	loop_error_msg = "incompatible protocols";
      }
      break;
    }

    case GledNS::MT_QueryFFID: {
      ISdebug(0, "Saturn::Accept FirstFreeIDQuery: reporting");

      ID_t wkid = mFireKing->GetSaturnID();
      TMessage m(GledNS::MT_QueryFFID);
      m << wkid;
      newsocket->Send(m);
      break;
    }

    case GledNS::MT_MEE_Connect: {
      ISdebug(0, "Saturn::Accept MEE_Connect ...");

      ZMirEmittingEntity* mee;
      mee = dynamic_cast<ZMirEmittingEntity*>(GledNS::StreamGlass(*msg));
      if(mee == 0) {
	loop_error = true;
	loop_error_msg = "MEE_Connect not followed by a MirEmittingEntity";
	break;
      }
      try {
	mSunQueen->handle_mee_connection(mee, newsocket);
      }
      catch(string exc) {
	TMessage m(GledNS::MT_MEE_ConnectionDenied);
	TString rep(exc.c_str());
	rep.Streamer(m);
	newsocket->Send(m);
	loop_error = true;
	loop_error_msg = exc;
	break;
      }
      loop_done = true;
      break;
    }

    case GledNS::MT_MEE_Authenticate: {
      ISdebug(0, "Saturn::Accept MEE_Authenticate ...");

      UInt_t conn_id;
      *msg >> conn_id;
      try {
	mSunQueen->handle_mee_authentication(conn_id, newsocket);
      }
      catch(string exc) {
	loop_error = true;
	loop_error_msg = exc;
	break;
      }
      delete newsocket;
      loop_done = true;
      break;
    }

      // Unknown
    default: {
      ISdebug(0, "Saturn::Accept got unknown message ... closing connection");
      loop_error = true;
      loop_error_msg = _eh + GForm("unknown message type %d", msg->What());
      break;
    }

    } // end switch message type

    delete msg;

  } // end top-loop

  //newsocket->SetOption(kNoBlock, 1);
}

void Saturn::finalize_moon_connection(SaturnInfo* si)
{
  {
    TMessage m(GledNS::MT_MEE_ConnectionGranted);
    lpZGlass_t kings; mGod->Copy(kings); kings.pop_back();
    m << si->GetSaturnID();
    m << (UInt_t) kings.size();
    mRulingLock.Lock();
    bool first = true;
    for(lpZGlass_i i=kings.begin(); i!=kings.end(); ++i) {
      ZKing* k = dynamic_cast<ZKing*>(*i);
      assert(k!=0);
      ZComet* s = k->MakeComet();
      s->Streamer(m);
      delete s;
      if(first) { // Stream also SunQueen
	s = mSunQueen->MakeComet();
	s->Streamer(m);
	delete s;
	first = false;
      }
    }
    mRulingLock.Unlock();
    si->hSocket->Send(m);
  }

  mMoonLock.Lock();
  mSelector.Lock();
  mSelector.fRead[si->hSocket->GetDescriptor()] = (void*)si->hSocket;
  mServerThread->Kill(GThread::SigUSR2);
  mSelector.Unlock();
  mSock2InfoHash.insert(pair<TSocket*, SocketInfo>
			(si->hSocket, SocketInfo(SocketInfo::OS_Moon, si)));
  mMoons.push_back(si);
  mSunQueen->add_reflector(si);
  si->hQueens.insert(mSunQueen);
  mMoonLock.Unlock();
}

void Saturn::finalize_eye_connection(EyeInfo* ei)
{
  {
    TMessage m(GledNS::MT_MEE_ConnectionGranted);
    m << (UInt_t)this << ei->GetSaturnID();
    ei->hSocket->Send(m);
  }

  mEyeLock.Lock();
  mSelector.Lock();
  mSelector.fRead[ei->hSocket->GetDescriptor()] = (void*)ei->hSocket;
  mServerThread->Kill(GThread::SigUSR2);
  mSelector.Unlock();
  mSock2InfoHash.insert(pair<TSocket*, SocketInfo>
	(ei->hSocket, SocketInfo(SocketInfo::OS_Eye, ei)));
  mEyes.push_back(ei);
  mEyeLock.Unlock();
}


/**************************************************************************/

Int_t Saturn::Manage(TSocket* sock) throw(string)
{
  ISdebug(8, GForm("Saturn::Manage Got sth from\t%s/%s:%d",
		   sock->GetInetAddress().GetHostName(),
		   sock->GetInetAddress().GetHostAddress(),
		   sock->GetInetAddress().GetPort()));

  int len, cnt = 0;
  TMessage *m;
  
  while(1) {
  start:
    len = sock->Recv(m);

    if(len == 0) { // closed connection
      ISmess("Saturn::Manage: Recv failed w/ code 0, other side closed connection");
      socket_closed(sock);
      break;
    }
    if(len == -1) { // error
      cerr << "Saturn::Manage: Recv failed w/ code -1\n";
      socket_closed(sock);
      break;
    }
    // Now all sockets unblocking ... so the followinf if irrelevant.
    // Would need ROOT support for a proper implementation (and several threads).
    if(len == -4) {
      ++cnt;
      cerr << "Saturn::Manage: Recv failed w/ code -4, cnt="<< cnt <<endl;
      if(cnt > 10) break;
      gSystem->Sleep(100);
      goto start;
    }

    ISdebug(8, GForm("Saturn::Manage len=%d, type=%d", len, m->What()));

    switch(m->What()) {

    case kMESS_STRING:
      // ????
      break;
      
    case GledNS::MT_Flare: 
    case GledNS::MT_Beam:
      {
	ZMIR mir(m); // Swallows m, deletes it and sets m to null.
	mir.ReadRoutingHeader();
	if(!bSunAbsolute && sock == mSaturnInfo->GetMaster()->hSocket) {
	  mir.Direction = ZMIR::D_Down;
	} else {
	  mir.Direction = ZMIR::D_Up;
	  hSock2SocketInfo_i i = mSock2InfoHash.find(sock);
	  if(i == mSock2InfoHash.end()) throw(string("Saturn::Manage unknown socket"));
	  switch(i->second.fWhat) {
	  case SocketInfo::OS_Moon:
	    // ?? Perhaps should check authenticity of caller
	    break;
	  case SocketInfo::OS_Eye:
	    // set caller as eyes don't do that (or even if they do ...)
	    mir.SetCaller( (i->second.get_eye()) );
	    break;
	  default:
	    throw(string("Saturn::Manage internal inconsistency"));
	  } // switch from
	}
	RouteMIR(mir);
	break;
      } // case Flare
    
    default:
      throw(string("Saturn::Manage unknown message type"));

    } // switch message type

    delete m;
    break;
  } // while sock

  return 0;
}

/**************************************************************************/
// Beam results & MIR posting
/**************************************************************************/

UInt_t Saturn::register_mir_result_request(GCondition* cond)
{
  mBeamReqHandleMutex.Lock();
  UInt_t req_handle = ++mLastBeamReqHandle;
  mBeamReqHandles.insert(pair<UInt_t, mir_rr_info>(req_handle, mir_rr_info(cond)));
  mBeamReqHandleMutex.Unlock();
  return req_handle;
}

ZMIR_RR* Saturn::query_mir_result(UInt_t req_handle)
{
  ZMIR_RR* ret = 0;
  mBeamReqHandleMutex.Lock();
  hReqHandle2MirRRInfo_i req = mBeamReqHandles.find(req_handle);
  if(req != mBeamReqHandles.end() && req->second.mir_rr != 0) {
    ret = req->second.mir_rr;
    mBeamReqHandles.erase(req);
  }
  mBeamReqHandleMutex.Unlock();
  return ret;
}

void Saturn::handle_mir_result(UInt_t req_handle, ZMIR* mirp)
{
  ZMIR_RR* res = new ZMIR_RR;
  *mirp >> res->MirRRBits;
  if(res->HasException()) {
    res->Exception.Streamer(*mirp);
  }
  if(res->HasResult()) {
    mirp->CopyToBuffer(*res);
    res->SetReadMode();
    res->SetBufferOffset(0);
  }

  mBeamReqHandleMutex.Lock();
  hReqHandle2MirRRInfo_i req = mBeamReqHandles.find(req_handle);
  if(req == mBeamReqHandles.end()) {
    mBeamReqHandleMutex.Unlock();
    delete res;
    throw(string("Saturn::handle_beam_result unresolved beam result request handle"));
  }
  req->second.mir_rr = res;
  req->second.cond->Lock();
  req->second.cond->Signal();
  req->second.cond->Unlock();
  mBeamReqHandleMutex.Unlock();
}

/**************************************************************************/

void Saturn::shoot_mir(auto_ptr<ZMIR>& mir, ZMirEmittingEntity* caller,
		       bool use_own_thread)
{
  // The auto_ptr<ZMIR> is released and should not be used afterwards.

  markup_posted_mir(*mir, caller);

  if(use_own_thread) {
    mir_router_ti* foo = new mir_router_ti(this, mir.release(), true);
    GThread bar((GThread_foo)(tl_MIR_Router), foo, true);
    bar.Spawn();
  } else {
    mMIRShootingCnd.Lock();
    mMIRShootingQueue.push_back(mir.release());
    mMIRShootingCnd.Signal();
    mMIRShootingCnd.Unlock();
  }
}

void Saturn::markup_posted_mir(ZMIR& mir, ZMirEmittingEntity* caller)
{
  // Writes header and caller infromation.
  // If caller==0 the owner of current thread is used.

  if(mir.What() == GledNS::MT_Flare)
    mir.Direction = ZMIR::D_Up;

  mir.SetCaller((caller==0) ? GThread::get_owner() : caller);
  mir.WriteHeader();
  mir.RewindToExecHeader();
}

void Saturn::shoot_mir_from_queue()
{
  // Routes one MIR from the ShootingQueue.
  // The MIR is deleted afterwards.

  mMIRShootingCnd.Lock();
  if(mMIRShootingQueue.empty())
    mMIRShootingCnd.Wait();
  auto_ptr<ZMIR> mir(mMIRShootingQueue.front());
  mMIRShootingQueue.pop_front();
  mMIRShootingCnd.Unlock();
  RouteMIR(*mir);
}

/**************************************************************************/

void Saturn::PostMIR(ZMIR& mir)
{
  // Routes a MIR in the current thread.
  // Usually called from Operators to publish some result.
  // If mir is to be re-used, do: mir.RewindToMIR() prior to posting it again.

  static string _eh("Saturn::PostMIR() ");
  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  markup_posted_mir(mir);
  try {
    RouteMIR(mir);
  }
  catch (string exc) {
    ISerr(_eh + "failed: " + exc);
  }
}

/**************************************************************************/


void Saturn::ShootMIR(auto_ptr<ZMIR>& mir, bool use_own_thread)
{
  // Accepts a MIR and shoots it into the system.
  // The auto_ptr<ZMIR> is released and should not be used afterwards.

  shoot_mir(mir, 0, use_own_thread);
}

ZMIR_RR* Saturn::ShootMIRWaitResult(auto_ptr<ZMIR>& mir, bool use_own_thread)
{
  // Accepts a MIR, shoots it into the system and waits for ResultReport.
  // The auto_ptr<ZMIR> is released and should not be used afterwards.

  GCondition result_cond(GMutex::recursive);
  UInt_t req_handle = register_mir_result_request(&result_cond);
  mir->SetResultReq(mSaturnInfo, req_handle);

  result_cond.Lock();
  shoot_mir(mir, 0, use_own_thread);  
  result_cond.Wait(); // !!!! timed-wait
  result_cond.Unlock();

  return query_mir_result(req_handle);
}

/**************************************************************************/

// sub function doing the actual construction ... and sending.
// otherwise will have three shitty stuffy

void Saturn::generick_shoot_mir_result(const Text_t* exc, TBuffer* buf)
{
  SaturnInfo* ret_addr = GThread::get_return_address();
  if(ret_addr == 0) return;
  GThread::set_return_address(0);
  UInt_t handle = GThread::get_return_handle();

  UChar_t bits = 0;
  if(exc) bits |= ZMIR_RR::B_HasException;
  if(buf) bits |= ZMIR_RR::B_HasResult;

  auto_ptr<ZMIR> mir( ret_addr->S_ReceiveBeamResult(handle) );
  *mir << bits;
  if(exc) {
    TString s(exc);
    s.Streamer(*mir);
  }
  if(buf) {
    mir->AppendBuffer(*buf);
  }

  mir->SetRecipient(ret_addr);
  mir->SetCaller(ret_addr); // The result pretends to be coming come from the caller itself
  mir->WriteHeader(); mir->RewindToExecHeader();
  mMIRShootingCnd.Lock();
  mMIRShootingQueue.push_back(mir.release());
  mMIRShootingCnd.Signal();
  mMIRShootingCnd.Unlock();
}

void Saturn::ShootMIRResult(TBuffer& b)
{
  // Makes a result MIR and sends it to the caller Saturn.
  // Should be called from a method called via a beamed MIR
  // containing a result request.

  generick_shoot_mir_result(0, &b);
}

/**************************************************************************/
// MIR handling
/**************************************************************************/

void Saturn::RouteMIR(ZMIR& mir)
{
  // Decides what to do with a MIR ... and does it.

  static string _eh("Saturn::RouteMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  GThread::set_return_address(0);
  GThread::set_return_handle(0);

  try {

    switch(mir.What()) {

    case GledNS::MT_Flare: {
      switch(mir.Direction) {
      case ZMIR::D_Up:
	if(mir.AlphaID < mKing->GetSaturnID())
	  ForwardMIR(mir, mSaturnInfo->GetMaster());
	else
	  UnfoldMIR(mir);
	break;
      case ZMIR::D_Down:
	UnfoldMIR(mir);
	break;
      default:
	throw(_eh + "unknown direction of a flare");
	break;
      }
      break;
    }

    case GledNS::MT_Beam: {
      if(!mir.RecipientID)
	throw(_eh + "can't determine recipient of a beam");
      if(mir.RecipientID == mSaturnInfo->GetSaturnID()) {
	UnfoldMIR(mir);
      } else {
	if(mir.Recipient == 0)
	  mir.Recipient = dynamic_cast<SaturnInfo*>(DemangleID(mir.RecipientID));
	SaturnInfo* route = mir.Recipient->hRoute;
	if(route == 0) {
	  route = FindRouteTo(mir.Recipient);
	  if(route == 0 || route->hSocket == 0) {
	    throw(_eh + "no route to recipient");
	  }
	  // The following line might cause a nightmare upon moon deletion.
	  // Or not ... hmmmmph ... it shouldn't ... it will be erased.
	  mir.Recipient->hRoute = route;
	}
	ForwardMIR(mir, route);
      }
      break;
    }

    default: throw(_eh + "unhandled message type");
    }

  }
  catch(string exc) {
    if(GThread::get_return_address()) {
      generick_shoot_mir_result(exc.c_str(), 0);
    }
    throw;
  }
}

void Saturn::UnfoldMIR(ZMIR& mir)
{
  // Prepares MIR for execution, does checks, proper broadcasting
  // and the execution itself.
  // Message type should be MT_Beam or MT_Flare.

  static string _eh("Saturn::UnfoldMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  bool is_shared_space;    // if shared_space and flare => mir should be broadcasted
  bool is_king  = false;
  bool is_flare = (mir.What() == GledNS::MT_Flare);

  bool is_moon_space = false;
  bool is_sun_space  = false;
  bool is_fire_space = false;

  try {
    mir.ReadExecHeader();
    mir.Demangle(this);
    if(!is_flare) {
      mir.DemangleRecipient(this);
    }
  }
  catch(string exc) {
    throw(_eh + "Context Demangling error: " + exc);
  }

  ZKing* king;
  if(mir.AlphaID < mKing->GetSaturnID())
    {
      is_shared_space = true;
      is_moon_space   = true;
      if(mir.Alpha->GetQueen() == 0) {
	king = dynamic_cast<ZKing*>(mir.Alpha);
	if(king != 0) {
	  is_king = true;
	} else {
	  throw(_eh + "Alpha wo/ queen (but not king) ... stalling");
	}
      } else {
	king = mir.Alpha->GetQueen()->GetKing();
      }
    }
  else if(mir.AlphaID >= mKing->GetSaturnID() &&
	  mir.AlphaID <= mKing->GetMaxID())
    {
      is_shared_space = true;
      is_sun_space    = true;
      king = mKing;
      if(mir.AlphaID == mKing->GetSaturnID()) is_king = true;
    }
  else if(mir.AlphaID >= mFireKing->GetSaturnID() &&
	  mir.AlphaID <= mFireKing->GetMaxID())
    {
      is_shared_space = false;
      is_fire_space   = true;
      king = mFireKing;
      if(mir.AlphaID == mFireKing->GetSaturnID()) is_king = true;
    }
  else
    {
      throw(_eh + "lens ID out of ruling range (should not happen)");
    }

  if(mir.HasResultReq() && (!is_flare || (is_flare && is_sun_space))) {
    mir.DemangleResultRecipient(this);
    GThread::set_return_address(mir.ResultRecipient);
    GThread::set_return_handle(mir.ResultReqHandle);
  }

  if(is_king) {
    mRulingLock.Lock();
    try {
      if(is_flare) {
	if(is_moon_space) {
	  BroadcastMIR(mir, mMoons);
	  ExecMIR(mir);
	} else {
	  king->BlessMIR(mir);
	  ExecMIR(mir);
	  if(is_shared_space && mir.SuppressFlareBroadcast == false) {
	    BroadcastMIR(mir, mMoons);
	  }
	  if(GThread::get_return_address()) generick_shoot_mir_result(0, 0);
	}
      } else {
	king->BlessMIR(mir);
	ExecMIR(mir);
	if(GThread::get_return_address()) generick_shoot_mir_result(0, 0);
      }
    }
    catch (string exc) {
      mRulingLock.Unlock();
      throw;
    }
    mRulingLock.Unlock();
  } else {
    ZQueen* queen = mir.Alpha->GetQueen();
    bool is_queen = (mir.Alpha == queen);
    if(is_queen) mRulingLock.Lock();
    queen->mExecMutex.Lock();
    try {
      if(is_flare) {
	if(is_moon_space) {
	  lpSaturnInfo_t& moons = is_queen ? mMoons : queen->mReflectors;
	  BroadcastMIR(mir, moons);
	  ExecMIR(mir);
	} else {
	  queen->BlessMIR(mir);
	  ExecMIR(mir);
	  if(is_shared_space && mir.SuppressFlareBroadcast == false) {
	    lpSaturnInfo_t& moons = is_queen ? mMoons : queen->mReflectors;
	    BroadcastMIR(mir, moons);
	  }
	  if(GThread::get_return_address()) generick_shoot_mir_result(0, 0);
	}
      } else {
	queen->BlessMIR(mir);
	ExecMIR(mir);
	if(GThread::get_return_address()) generick_shoot_mir_result(0, 0);
      }
    }
    catch (string exc) {
      queen->mExecMutex.Unlock();
      if(is_queen) mRulingLock.Unlock();
      EyeInfo* ei = dynamic_cast<EyeInfo*>(mir.Caller);
      if(ei && (!is_flare || mir.Alpha->IsSunSpace())) {
	SP_MIR_BEAM(ei->GetMaster(), ei, Error,
		    GForm("Saturn::UnfoldMIR exception: %s", exc.c_str()));
      }
      throw;
    }
    queen->mExecMutex.Unlock();
    if(is_queen) mRulingLock.Unlock();
  }
} 

void Saturn::ExecMIR(ZMIR& mir) throw(string)
{
  static string _eh("Saturn::ExecMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  if(mir.IsWriting()) mir.RewindToData();

  GledNS::LibSetInfo* lsi = GledNS::FindLibSetInfo(mir.Lid);
  if(lsi == 0)
    throw(string(GForm("Saturn::ExecMIR can't demangle lib ID=%u", (unsigned int)mir.Lid)));
  
  ZMirEmittingEntity*  ex_mee = GThread::get_owner();
  GThread::set_owner(mir.Caller);

  ZGlass* lens = mir.Alpha;
  lens->mExecMutex.Lock();
  lens->mMir = &mir;
  try {
    (lsi->fLME_Foo)(lens, mir);
  }
  catch(string exc) {
    lens->mMir = 0;
    lens->mExecMutex.Unlock();
    GThread::set_owner(ex_mee);
    throw;
  }
  lens->mMir = 0;
  lens->mExecMutex.Unlock();
  GThread::set_owner(ex_mee);
}

void Saturn::ForwardMIR(ZMIR& mir, SaturnInfo* route)
{
  // Forwards a MIR to Saturn given by route.

  static string _eh("Saturn::ForwardMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  assert(route->hSocket != 0);
  bool is_reading = mir.IsReading();
  if(is_reading) mir.Forward();
  bool upwards_p = (route == mSaturnInfo->GetMaster());
  if(upwards_p) mMasterLock.Lock(); else mMoonLock.Lock();

  // cout <<"Saturn::ForwardMIR routing to "<< route->GetName() <<endl;
  Int_t ret = route->hSocket->Send(mir);
  // cout <<"Saturn::ForwardMIR ret="<< ret <<endl;
  if(ret == -1) socket_closed(route->hSocket);

  if(upwards_p) mMasterLock.Unlock(); else mMoonLock.Unlock();
  if(is_reading) mir.RewindToData();
}

void Saturn::BroadcastMIR(ZMIR& mir, lpSaturnInfo_t& moons)
{
  // Broadcasts MIR to reflecting Moons specified in socks.

  static string _eh("Saturn::BroadcastMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  if(moons.empty()) return;
  bool was_reading = mir.IsReading();
  if(was_reading) mir.Forward();
  mMoonLock.Lock();
  ISdebug(9, GForm("Saturn::BroadcastMIR sending to %d moon(s)", moons.size()));
  lpSaturnInfo_i i = moons.begin();
  while(i != moons.end()) {
    if((*i)->hSocket) {
      Int_t ret = (*i)->hSocket->Send(mir);
      lpSaturnInfo_i j = i++;
      if(ret < 0) {
	// Should be more gracefull !!!!?
	wipe_moon(*j, true);
      }
    }
  }
  mMoonLock.Unlock();
  if(was_reading) mir.RewindToData();
}

void Saturn::BroadcastBeamMIR(ZMIR& mir, lpSaturnInfo_t& moons)
{
  // Broadcasts MIR to reflecting Moons specified in socks.
  // Specifically sets each SaturnInfo to be the recipient.

  static string _eh("Saturn::BroadcastMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  if(moons.empty()) return;
  bool was_reading = mir.IsReading();
  if(was_reading) mir.Forward();
  mMoonLock.Lock();
  ISdebug(9, GForm("Saturn::BroadcastMIR sending to %d moon(s)s", moons.size()));
  lpSaturnInfo_i i = moons.begin();
  while(i != moons.end()) {
    if((*i)->hSocket) {
      mir.SetRecipient(*i);
      Int_t ret = (*i)->hSocket->Send(mir);
      lpSaturnInfo_i j = i++;
      if(ret < 0) {
	// Should be more gracefull !!!!
	wipe_moon(*j, true);
      }
    }
  }
  mMoonLock.Unlock();
  if(was_reading) mir.RewindToData();
}

/**************************************************************************/
// Protected & less pleasant
/**************************************************************************/

int Saturn::start_server()
{
  mServerSocket = new TServerSocket(mSaturnInfo->GetServerPort(), kTRUE, 4);
  if( !mServerSocket->IsValid() ) {
    int err = 10 - mServerSocket->GetErrorCode();
    ISerr("Saturn::start_server Can't create ServerSocket ... Dying");
    return err;
  }
  mSelector.fRead[mServerSocket->GetDescriptor()] = (void*)mServerSocket;
  ISdebug(2, GForm("Saturn::start_server starting server, port=%d",
		   mSaturnInfo->GetServerPort()));
  
  mServerThread = new GThread((GThread_foo)tl_SaturnFdSucker, this, false);
  _server_startup_cond.Lock();
  if( mServerThread->Spawn() ) {
    ISerr(GForm("Saturn::start_server Can't create server thread ... dying. errno=%d", errno));
    return 1;
  }
  _server_startup_cond.Wait();
  _server_startup_cond.Unlock();
  ISdebug(2, "Saturn::start_server Started Server thread");
  return 0;
}

int Saturn::stop_server()
{
  if(mSaturnInfo == 0) return 0;

  if(mServerThread) {
    mServerThread->Cancel();
    mServerThread->Join();
    delete mServerThread;
    mServerThread = 0;
  }

  if(mServerSocket) {
    mServerSocket->Close();
    delete mServerSocket;
    mServerSocket = 0;
  }

  // Need some locking here ?!!
  if(!bSunAbsolute && mSaturnInfo->GetMaster()) {
    mSaturnInfo->GetMaster()->hSocket->Close();
  }
  for(lpEyeInfo_i ei=mEyes.begin(); ei!=mEyes.end(); ++ei) {
    (*ei)->hSocket->Close();
  }
  for(lpSaturnInfo_i si=mMoons.begin(); si!=mMoons.end(); ++si) {
    (*si)->hSocket->Close();    
  }

  return 0;
}

/**************************************************************************/

int Saturn::start_mir_shooter()
{
  mMIRShootingThread = new GThread((GThread_foo)tl_MIR_Shooter, this, false);
  if( mMIRShootingThread->Spawn() ) {
    ISerr(GForm("Saturn::start_mir_shooter Can't create a MirShooter thread ... dying. errno=%d", errno));
    return 1;
  }
  ISdebug(2, "Saturn::start_mir_shooter Started MirShooter thread");
  return 0;
}

int Saturn::stop_mir_shooter()
{
  if(mMIRShootingThread) {
    mMIRShootingThread->Cancel();
    mMIRShootingThread->Join();
    delete mMIRShootingThread;
    mMIRShootingThread = 0;
  }
  return 0;
}

/**************************************************************************/
/**************************************************************************/

void Saturn::socket_closed(TSocket* sock)
{
  if(!bSunAbsolute && sock == mSaturnInfo->GetMaster()->hSocket) {
    // Here should take some drastic measures.
    // Retake sun control ... or sth.
    // For now die miserably ...
    assert(123==321);
    return;
  }

  hSock2SocketInfo_i i = mSock2InfoHash.find(sock);
  if(i == mSock2InfoHash.end()) {
    ISerr("Saturn::WipeMoonOrEye can't find socket ...");
    return;
  }
  switch(i->second.fWhat) {
  case SocketInfo::OS_Moon:
    wipe_moon(i->second.get_moon(), true);
    break;
  case SocketInfo::OS_Eye: {
    wipe_eye(i->second.get_eye(), true);
    break;
  }
  default: {
    ISerr("Saturn::WipeMoonOrEye unknown socket ID ...");
  }
  } // switch
}

void Saturn::wipe_moon(SaturnInfo* moon, bool notify_sunqueen_p)
{
  // Removes moon from list of moons and notifies the SunQueen if
  // notify_sunqueen_p is true.
  // Also removes the moon from all queens that are reflecting to it.

  ISmess(GForm("Saturn::wipe_moon closing connection for Moon %s",
		   moon->GetName()));
  if(moon->hSocket == 0) {
    // Hmmpsh ... shouldn't really happen ... or just maybe.
    ISmess("Saturn::wipe_moon socket already wiped ... stalling");
    return;
  }

  mMoonLock.Lock();

  if(mServerThread) mServerThread->Kill(GThread::SigUSR2);
  mSelector.Lock();
  mSelector.fRead.erase(moon->hSocket->GetDescriptor());
  mSelector.Unlock();

  hSock2SocketInfo_i h = mSock2InfoHash.find(moon->hSocket);
  if(h != mSock2InfoHash.end()) {
    mSock2InfoHash.erase(h);
  } else {
    ISerr(GForm("Saturn::wipe_moon %s not found in hash", moon->GetName()));
  }

  lpSaturnInfo_i l = find(mMoons.begin(), mMoons.end(), moon);
  if(l != mMoons.end()) {
    mMoons.erase(l);
  } else {
    ISerr(GForm("Saturn::wipe_moon %s not found in list", moon->GetName()));
  }
  for(SaturnInfo::spZQueen_i q=moon->hQueens.begin(); q!=moon->hQueens.end(); ++q) {
    (*q)->remove_reflector(moon);
  }
  
  delete moon->hSocket; moon->hSocket = 0;
  moon->hRoute = 0; moon->hQueens.clear();

  mMoonLock.Unlock();

  if(notify_sunqueen_p) {
    auto_ptr<ZMIR> mir( mSunQueen->S_CremateMoon(moon) );
    mir->SetCaller(mSaturnInfo);
    mir->SetRecipient(mSunInfo);
    mir->WriteHeader(); mir->RewindToExecHeader();
    RouteMIR(*mir);
  }
}

void Saturn::wipe_eye(EyeInfo* eye, bool notify_sunqueen_p)
{
  ISmess(GForm("Closing connection for Eye %s", eye->GetName()));
  if(eye->hSocket == 0) {
    // Hmmpsh ... shouldn't really happen ... or just maybe.
    ISmess(GForm("Saturn::wipe_eye socket already wiped ... stalling"));
    return;
  }

  mEyeLock.Lock();

  if(mServerThread) mServerThread->Kill(GThread::SigUSR2);
  mSelector.Lock();
  mSelector.fRead.erase(eye->hSocket->GetDescriptor());
  mSelector.Unlock();

  hSock2SocketInfo_i h = mSock2InfoHash.find(eye->hSocket);
  if(h != mSock2InfoHash.end()) {
    mSock2InfoHash.erase(h);
  } else {
    ISerr(GForm("Saturn::wipe_eye %s not found in hash", eye->GetName()));
  }

  lpEyeInfo_i l = find(mEyes.begin(), mEyes.end(), eye);
  if(l != mEyes.end()) {
    mEyes.erase(l);
  } else {
    ISerr(GForm("Saturn::wipe_eye %s not found in list", eye->GetName()));
  }
  
  delete eye->hSocket; eye->hSocket = 0;

  mEyeLock.Unlock();

  if(notify_sunqueen_p) {
    auto_ptr<ZMIR> mir( mSunQueen->S_CremateEye(eye) );
    mir->SetCaller(mSaturnInfo);
    mir->SetRecipient(mSunKing->mSaturnInfo);
    mir->WriteHeader(); mir->RewindToExecHeader();
    RouteMIR(*mir);
  }
}

/**************************************************************************/

void Saturn::fix_fire_king_id(SaturnInfo* si)
{
  if(si->mSunSpaceSize == 0 || (si->mKingID + si->mSunSpaceSize) < si->mKingID) {
    si->mSunSpaceSize = ID_t(-1) - si->mKingID;
    si->mSunSpaceSize >>= 3;
  }
  si->mFireKingID = si->mKingID + si->mSunSpaceSize;
  ISmess(GForm("Saturn::fix_fire_king_id sun_space(beg=%5.3f siz=%5.3f) "
	       "fire-space: beg=%5.3f siz=%5.3f",
	       double(si->mKingID)/ID_t(-1),
	       double(si->mFireKingID - si->mKingID)/ID_t(-1),
	       double(si->mFireKingID)/ID_t(-1),
	       double(ID_t(-1) - si->mFireKingID)/ID_t(-1)));
}

void Saturn::create_kings(const char* king, const char* whore_king)
{
  // Local King
  mKing = new ZKing(mSaturnInfo->mKingID + 1, mSaturnInfo->mFireKingID - 1,
		    king);
  mKing->mSaturnInfo = mSaturnInfo;
  mKing->mLightType = ZKing::LT_Sun;
  Enlight(mKing, mSaturnInfo->mKingID);
  mGod->Add(mKing);

  // Fire King
  mFireKing = new ZKing(mSaturnInfo->mFireKingID + 1, MAX_ID-1, whore_king);
  mFireKing->mSaturnInfo = mSaturnInfo;
  mFireKing->mLightType = ZKing::LT_Fire;
  mFireKing->bFireKing = true;
  Enlight(mFireKing, mSaturnInfo->mFireKingID);
  mGod->Add(mFireKing);
  mFireQueen = new ZQueen(1024, "FireQueen", "Enchantress of ChaOss");
  mFireKing->Enthrone(mFireQueen);
  mFireQueen->SetMapNoneTo(ZMirFilter::R_Allow);
}

void Saturn::arrival_of_kings(TMessage* m)
{
  *m >> mSaturnInfo;
  UInt_t nk; *m >> nk;
  ISmess(GForm("Saturn::arrival_of_kings %d king(s), len=%d",
	       nk, m->BufferSize()));
  for(UInt_t i=0; i<nk; ++i) {
    ZComet c; c.Streamer(*m);
    c.bWarnOn = false; c.bVerbose = false;
    c.RebuildGraph();
    ZKing* k = c.GetKing();
    assert(k!=0);
    Reflect(k);
    k->mLightType = ZKing::LT_Moon;
    lpZGlass_t queens; k->Copy(queens);
    for(lpZGlass_i j=queens.begin(); j!=queens.end(); ++j) {
      ZQueen* q = dynamic_cast<ZQueen*>(*j);
      assert(q!=0);
      q->mKing = k;
      Reflect(q); q->mQueen = q;
      Reflect(q->mDeps); q->mDeps->mQueen = q;
    }
    if(i == 0) { // This is SunKing && SunQueen follows
      mSunKing  = k;
      mSunQueen = dynamic_cast<ZSunQueen*>(queens.front());
      assert(mSunQueen!=0);

      ZComet t; t.Streamer(*m); t.UseSaturn(this);
      mSaturnInfo = dynamic_cast<SaturnInfo*>(t.FindID((ID_t)mSaturnInfo));
      assert(mSaturnInfo != 0);
      //t.bWarnOn = false; t.bVerbose = false;
      t.RebuildGraph();
      mSunQueen->UnfoldFrom(t);
      // Fix mSunInfo and SunKing's SaturnInfo pointer
      mSunInfo = mSunKing->mSaturnInfo = mSunQueen->mSunInfo;
    }
    mGod->Add(k);
  }
}

/**************************************************************************/
