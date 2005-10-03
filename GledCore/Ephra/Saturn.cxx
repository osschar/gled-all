// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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
#include <Glasses/ZFireKing.h>
#include <Glasses/ZSunQueen.h>
#include <Glasses/ZQueen.h>
#include <Glasses/ZFireQueen.h>
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

  GMutex	_detached_mir_mgmnt_lock;

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
  _server_startup_cond.Wait();
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
  GThread::DeleteIfDetached();
  GThread::Exit();
  return 0;
}

void* Saturn::tl_MIR_Router(mir_router_ti* arg)
{
  // Thread foo for routing of a single MIR.
  // If the thread is detached, this method also deletes the GThread.

  static const Exc_t _eh("Saturn::tl_MIR_Router ");

  GThread::setup_tsd(arg->sat->GetSaturnInfo());

  auto_ptr<ZMIR> mir(arg->mir);
  try {
    arg->sat->RouteMIR(mir);
    if(mir->RequiresResult) arg->sat->generick_shoot_mir_result(*mir, 0, 0);
  }
  catch(Exc_t& exc) {
#ifdef DEBUG
    arg->sat->report_mir_post_demangling_error(*mir, _eh + "MIR execution failed: " + exc);
#else
    arg->sat->report_mir_post_demangling_error(*mir, exc + " [MirRouter]");
#endif
  }

  if(!arg->delete_mir)
    mir.release();
  delete arg;
  
  GThread::DeleteIfDetached();
  GThread::Exit();
  return 0;
}


void* Saturn::tl_MIR_DetachedExecutor(mir_router_ti* arg)
{
  // Thread foo for execution of a single *detached* MIR.

  static const Exc_t _eh("Saturn::tl_MIR_DetachedExecutor ");

  // These are the defaults anyway ...
  GThread::SetCancelState(GThread::CS_Enable);
  GThread::SetCancelType(GThread::CT_Deferred);

  GThread::setup_tsd(arg->sat->GetSaturnInfo());

  arg->self = GThread::TSDSelf(); // Needed in clean-up.

  GTHREAD_CU_PUSH;
  
  {
    ISdebug(1, GForm("%sregistering a detached thread %p to '%s'.",
		     _eh.Data(), arg->self, arg->mir->Alpha->GetName()));
    GMutexHolder mh(_detached_mir_mgmnt_lock);
    arg->sat->mDetachedThreadsHash[arg->mir->Alpha].push_back(arg->self);
  }

  auto_ptr<ZMIR> mir(arg->mir);
  try {
    ISdebug(1, GForm("%snow execing mir.", _eh.Data()));
    arg->sat->ExecMIR(mir, false);
    if(mir->RequiresResult) arg->sat->generick_shoot_mir_result(*mir, 0, 0);
    ISdebug(1, GForm("%smir exec done.", _eh.Data()));
  }
  catch(Exc_t& exc) {
#ifdef DEBUG
    arg->sat->report_mir_post_demangling_error(*mir, _eh + "MIR execution failed: " + exc);
#else
    arg->sat->report_mir_post_demangling_error(*mir, exc + " [DetExe]");
#endif
  }

  GThread::TSDSelf()->Detach();

  mir.release();

  GThread::DeleteIfDetached();
  GThread::Exit();

  GTHREAD_CU_POP;

  return 0;
}

void Saturn::tl_MIR_DetachedCleanUp(mir_router_ti* arg)
{
  static const Exc_t _eh("Saturn::tl_MIR_DetachedCleanUp ");

  {
    ISdebug(1, GForm("%sunregistering a detached thread %p from '%s'.",
		     _eh.Data(), arg->self, arg->mir->Alpha->GetName()));
    GMutexHolder mh(_detached_mir_mgmnt_lock);
    hpZGlass2lpGThread_i i = arg->sat->mDetachedThreadsHash.find(arg->mir->Alpha);
    if(i == arg->sat->mDetachedThreadsHash.end()) {
      ISerr(_eh + "entry for " + arg->mir->Alpha->Identify() + " not found in mDetachedThreadsHash.");
      return;
    }
    int l1 = i->second.size();
    i->second.remove(arg->self);
    int l2 = i->second.size();
    ISdebug(1, GForm("%sl1=%d, l2=%d.", _eh.Data(), l1, l2));
    if(l2 == 0)
      arg->sat->mDetachedThreadsHash.erase(i);
  } 

  if(arg->delete_mir) delete arg->mir;
  delete arg;  
}

/**************************************************************************/

void* Saturn::tl_MIR_Shooter(Saturn* s)
{
  // Thread foo executing Saturn::mir_shooter().

  GThread::SetCancelState(GThread::CS_Enable);
  GThread::SetCancelType(GThread::CT_Deferred);

  GThread::setup_tsd(s->GetSaturnInfo());

  s->mir_shooter();

  GThread::Exit(0);
  return 0;
}

void* Saturn::tl_Delayed_MIR_Shooter(Saturn* s)
{
  // Thread foo executing Saturn::dealyed_mir_shooter().

  GThread::SetCancelState(GThread::CS_Enable);
  GThread::SetCancelType(GThread::CT_Deferred);

  GThread::setup_tsd(s->GetSaturnInfo());

  s->delayed_mir_shooter();

  GThread::Exit(0);
  return 0;
}

void* Saturn::tl_Ray_Emitter(Saturn* s)
{
  // Thread foo executing Saturn::ray_emitter().

  GThread::SetCancelState(GThread::CS_Enable);
  GThread::SetCancelType(GThread::CT_Deferred);

  GThread::setup_tsd(s->GetSaturnInfo());

  s->ray_emitter();

  GThread::Exit(0);
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
  mMIRShootingCnd(GMutex::recursive), mDelayedMIRShootingCnd(GMutex::recursive),
  bAcceptsRays(false), mRayEmittingCnd()
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

TString Saturn::HandleClientSideSaturnHandshake(TSocket*& socket)
{
  // Handles handshake with a Saturn on a newly opened socket.
  // Returns greeting TString.

  static const Exc_t _eh("Saturn::HandleClientSideSaturnHandshake ");

  TInetAddress ia = socket->GetInetAddress();
  if(!socket->IsValid()) {
    delete socket; socket = 0;
    throw(_eh + GForm("opening socket to %s:%d failed.",
		      ia.GetHostName(), ia.GetPort()));
  }

  // Receive greeting
  int ml; char buf[256];
  ml = socket->RecvRaw(buf, 255, kDontBlock);
  if(ml <= 0) {
    delete socket; socket = 0;
    throw(_eh + GForm("handshake with %s:%d failed: len=%d.",
		      ia.GetHostName(), ia.GetPort(), ml));
  }
  buf[ml] = 0;
  return TString(buf);
}

TMessage* Saturn::HandleClientSideMeeConnection(TSocket* socket, ZMirEmittingEntity* mee)
{
  // Sends request for instantiation of mee via socket.
  // If authentication is required it is handled via
  // ZSunQueen::HandleClientSideAuthentication(). The required identity
  // is deduced from the TString ZMirEmittingEntity::mLogin.

  static const Exc_t _eh("Saturn::HandleClientSideMeeConnection "); // caller should prefix the exception

  { // Send the desired Identity & Mee
    TMessage creq(GledNS::MT_MEE_Connect);
    GledNS::StreamLens(creq, mee);
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
    catch(Exc_t& exc) {
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

  static const Exc_t _eh("Saturn::Create ");

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

  if(start_server() || start_shooters()) {
    exit(1);
  }

  if(GThread::get_self() == 0)
    GThread::wrap_and_register_self(mSaturnInfo);

  try {
    mSunQueen->attach_primary_identity(mSaturnInfo);
  }
  catch(Exc_t& exc) {
    cerr << _eh + "exception: "<< exc <<endl;
    exit(1);
  }

  // Allow server thread to accept connections:
  _server_startup_cond.Lock();
  _server_startup_cond.Signal();
  _server_startup_cond.Unlock();
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

  static const Exc_t _eh("Saturn::Connect ");

  bSunAbsolute = false;

  TSocket* sock = new TSocket(si->GetMasterName(), si->GetMasterPort());
  try {
    TString greeting = Saturn::HandleClientSideSaturnHandshake(sock);
    ISmess(_eh + greeting);
  }
  catch(Exc_t& exc) {
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
      throw(_eh + GForm("protocol exchange failed; len=%d.",rl));
    }

    if(r->What() == GledNS::MT_GledProtocol) {
      Int_t sproto; *r >> sproto;
      ISmess(_eh + GForm("server protocol=%d.", sproto));
    } else if(r->What() == GledNS::MT_ProtocolMismatch) {
      Int_t sproto; *r >> sproto;
      delete sock; delete r;
      throw(_eh + GForm("protocol mismatch: server=%d, client=%d.",
			sproto, s_Gled_Protocol_Version));
    } else {
      Int_t mt = r->What();
      delete sock; delete r;
      throw(_eh + GForm("unknown message type %d.", mt));
    }
  }

  { // QueryFFId
    TMessage m(GledNS::MT_QueryFFID);
    sock->Send(m);
    TMessage* r;
    int rl = sock->Recv(r);
    if(rl <= 0) {
      delete sock;
      throw(_eh + "handshake failed (3).");
    }

    if(r->What() == GledNS::MT_QueryFFID) {
      ID_t ffid; *r >> ffid;
      ISmess(_eh + GForm("got first free id=%d.", ffid));

      si->mKingID = ffid;
      if(si->mKingID == 0) throw(_eh + "failed FFID query.");
      fix_fire_king_id(si);

    } else {
      Int_t mt = r->What();
      delete sock; delete r;
      throw(_eh + GForm("unknown message type %d.", mt));
    }
  }

  {
    TMessage* m;
    try {
      m = HandleClientSideMeeConnection(sock, si);
    }
    catch(Exc_t& exc) {
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

  if(start_server() || start_shooters()) {
    exit(1);
  }

  if(GThread::get_self() == 0)
    GThread::wrap_and_register_self(mSaturnInfo);

  // Issue requests for mandatory queens.
  lpZGlass_t kings; mGod->CopyList(kings); kings.pop_back(); kings.pop_back();
  for(lpZGlass_i i=kings.begin(); i!=kings.end(); ++i) {
    ZKing* k = dynamic_cast<ZKing*>(*i);
    lpZGlass_t queens; k->CopyList(queens);
    for(lpZGlass_i j=queens.begin(); j!=queens.end(); ++j) {
      ZQueen* q = dynamic_cast<ZQueen*>(*j);
      q->mKing = k;
      if(q->GetMandatory() && ! q->GetRuling()) {
	mFireKing->RequestQueenMirroring(q);
      }
    }
  }
  
  // Allow server thread to accept connections:
  _server_startup_cond.Lock();
  _server_startup_cond.Signal();
  _server_startup_cond.Unlock();

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

  static const Exc_t _eh("Saturn::Shutdown ");

  ISmess(_eh + "commencing.");

  ISmess(_eh + "stopping ChaItOss (thread manager).");
  mChaItOss->Shutdown();

  ISmess(_eh + "stopping MIR shooters.");
  stop_shooters();
  ISmess(_eh + "stopping server thread.");
  stop_server();

  ISmess(_eh + "stopping services.");
  if(pZHistoManager) {
    pZHistoManager->Write();
    pZHistoManager->Close();
    delete pZHistoManager; pZHistoManager = 0;
  }

  ISmess(_eh + "done.");

}

/**************************************************************************/

void Saturn::LockMIRShooters(bool wait_until_queue_empty)
{
  mMIRShootingCnd.Lock();
  if(wait_until_queue_empty) {
    while(1) {
      if(mMIRShootingQueue.empty()) break;
      // printf("Saturn::LockMIRShooters MIRShootingQueue non-empty ... delaying.\n");
      mMIRShootingCnd.Wait();
    }
  }
  mMIRShooterRoutingLock.Lock();
  mMIRShootingCnd.Unlock();

  mDelayedMIRShootingCnd.Lock();
}

void Saturn::UnlockMIRShooters()
{
  mDelayedMIRShootingCnd.Unlock();
  //mMIRShootingCnd.Unlock();
  mMIRShooterRoutingLock.Unlock();
}


/**************************************************************************/
// Enlight/Reflect/Freeze/Endark of lenses
/**************************************************************************/

void Saturn::Enlight(ZGlass* glass, ID_t id) throw(Exc_t)
{
  // Inserts glass into hash with key id.
  // Sets Saturn related glass members and calls ZGlass::AdEnlightenment().

  static const Exc_t _eh("Saturn::Enlight ");

  if(glass == 0)
    throw(_eh + "glass=0; stalling ...");

  if(id == 0)
    throw(_eh + "id=0; stalling ...");


  mIDLock.Lock();
  hID2pZGlass_i i = mIDHash.find(id);
  if(i != mIDHash.end()) {
    mIDLock.Unlock();
    throw(_eh + GForm("id=%u already used by '%s' (new is '%s'; stalling...",
		      id, i->second->GetName(), glass->GetName()));
  }
  mIDHash[id] = glass;
  mIDLock.Unlock();

  // Infrom the new lens
  glass->mSaturnID = id;
  glass->mSaturn = this;
  ISdebug(2,GForm("%sfor %s, id=%u", _eh.Data(), glass->GetName(), id));
  // Now let the newglass call its initialization foos
  glass->AdEnlightenment();
}

void Saturn::Reflect(ZGlass* glass) throw(Exc_t)
{
  // Enlights glass with a key taken from glass->mSaturnID.
  // Used for ZComet unpacking on Moons (hence Reflect).

  static const Exc_t _eh("Saturn::Reflect ");

  if(glass->mSaturnID == 0) throw (_eh + "called w/ id 0.");
  Enlight(glass, glass->mSaturnID);
}

void Saturn::Freeze(ZGlass* lens) throw(Exc_t)
{
  // Stops all detached MIR threads for lens.

  static const Exc_t _eh("Saturn::Freeze ");

  if(lens==0)
    throw(_eh + "lens=0; stalling.");

  ISdebug(2,GForm("%sfor %s, id=%u", _eh.Data(), lens->GetName(), lens->GetSaturnID()));

  int ok = 0, failed = 0;
  // !! Could be rewritten into two loops: Cancel, then Join those that
  // were successfully cancelled.
  while (true) {
    GThread* thr = 0;
    {
      GMutexHolder mh(_detached_mir_mgmnt_lock);
      hpZGlass2lpGThread_i i = mDetachedThreadsHash.find(lens);
      if (i == mDetachedThreadsHash.end())
	break;
      if( i->second.empty() ) {
	mDetachedThreadsHash.erase(i);
	break;
      }
      thr = i->second.back();
      i->second.pop_back();
    }
    ISdebug(2, GForm("%sattempting cancellation of a detached MIR thread of lens '%s'.",
		     _eh.Data(), lens->GetName()));
    int retc = thr->Cancel();
    ISdebug(2, GForm("%scancellation of thread of lens '%s' returned %d.",
		     _eh.Data(), lens->GetName(), retc));
    if(retc) {
      ISerr(_eh + "having problems cancelling a detached thread of " + lens->GetName() + ".");
      ++failed;
    } else {
      ISdebug(2, GForm("%sattempting join on a detached MIR thread of lens '%s'.",
		       _eh.Data(), lens->GetName()));
      int retj = thr->Join();
      ISdebug(2, GForm("%sjoin on thread of lens '%s' returned %d.",
		       _eh.Data(), lens->GetName(), retj));
      if(retj) {
	ISerr(_eh + "having problems joining a detached thread of " + lens->GetName() + ".");
	++failed;
      } else {
	delete thr;
	++ok;
      }
    }
  }
  if(ok > 0 || failed > 0) {
    ISdebug(1, GForm("%ssuccessfully canceled %d (failed for %d) detached MIR threads of lens '%s'.",
		     _eh.Data(), ok, failed, lens->GetName()));
  }
}

void Saturn::Endark(ZGlass* lens) throw(Exc_t)
{
  static const Exc_t _eh("Saturn::Endark ");

  if(lens==0)
    throw(_eh + "lens=0; stalling.");

  ISdebug(2,GForm("%sfor %s, id=%u", _eh.Data(), lens->GetName(), lens->GetSaturnID()));

  // Hack self
  mIDLock.Lock();
  hID2pZGlass_i i;
  if((i=mIDHash.find(lens->mSaturnID)) == mIDHash.end() || i->second != lens) {
    mIDLock.Unlock();
    throw(_eh + "id/lens mismatch; stalling.");
  }
  mIDHash.erase(i);
  mIDLock.Unlock();
  lens->AdEndarkenment();
  lens->mSaturnID = 0;
  lens->mSaturn = 0;
}

/**************************************************************************/
//
/**************************************************************************/

// Freezing not really needed anymore. Now solved via RulingLock,
// ZGlass::ReadLock and ZQueen::SubjectWriteLock.

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

Bool_t Saturn::IsMoon(SaturnInfo* si)
{
  mMoonLock.Lock();
  lpSaturnInfo_i l = find(mMoons.begin(), mMoons.end(), si);
  Bool_t ret = (l != mMoons.end());
  mMoonLock.Unlock();
  return ret;
}

void Saturn::CopyMoons(lpSaturnInfo_t& list)
{
  mMoonLock.Lock();
  copy(mMoons.begin(), mMoons.end(), back_inserter(list));
  mMoonLock.Unlock();
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
    catch(Exc_t& exc) {
      ISerr(GForm("Saturn::SockSuck caught exception: %s", exc.Data()));
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

  static const Exc_t _eh("Saturn::AcceptWrapper ");

  if(!newsocket) {
    ISerr(_eh + "Accept socket failed.");
    return;
  }

  try {
    Accept(newsocket);
  }
  catch(Exc_t& exc) {
    // Perhaps should report to someone
    ISerr(_eh + " exception: " + exc.Data());
  }
}

void Saturn::Accept(TSocket* newsocket) throw(Exc_t)
{
  // Accepts connection from an Eye/Moon or handles FirstFreeID query.
  // Locks/Suspends all executions upon
  // adding fd to Selector and sending scene to the Moon.
  // Runs in a detached thread (called from tl_SaturnAcceptor) ... 
  // perhaps should timeout.
  // Handles one connection at a time.

  static const Exc_t _eh("Saturn::Accept ");

  ISdebug(0, _eh + GForm("Connection from '%s'.",
		   newsocket->GetInetAddress().GetHostName()));

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
  TString loop_error_msg;

  while(!loop_done) {

    if(loop_error) {
      delete newsocket;
      throw(_eh + loop_error_msg);
    }

    int ret = sel.Select();

    if(ret <= 0) {
      // do clean-up
      // make descriptive message
      TString err_msg = (ret == 0) ? "connection timeout." : "select error.";

      delete newsocket;
      throw(_eh + err_msg);
    }

    TMessage* msg;
    int len = newsocket->Recv(msg);
    if(len <= 0) {
      delete newsocket;
      if(len == 0) {
	ISdebug(0, _eh +"other side closed connection.");
	return;
      } else {
	throw(_eh + GForm("error receiving message (len=%d).", len));
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
	loop_error_msg = "incompatible protocols.";
      }
      break;
    }

    case GledNS::MT_QueryFFID: {
      ISdebug(0, _eh + "FirstFreeIDQuery: reporting.");

      ID_t wkid = mFireKing->GetSaturnID();
      TMessage m(GledNS::MT_QueryFFID);
      m << wkid;
      newsocket->Send(m);
      break;
    }

    case GledNS::MT_MEE_Connect: {
      ISdebug(0, _eh + "MEE_Connect ...");

      ZMirEmittingEntity* mee;
      mee = dynamic_cast<ZMirEmittingEntity*>(GledNS::StreamLens(*msg));
      if(mee == 0) {
	loop_error = true;
	loop_error_msg = "MEE_Connect not followed by a MirEmittingEntity.";
	break;
      }
      ISmess(_eh + GForm("MEE_Connect (type='%s', name='%s', host='%s').",
			 mee->IsA()->GetName(), mee->GetName(),
			 newsocket->GetInetAddress().GetHostName()));
      try {
	mSunQueen->handle_mee_connection(mee, newsocket);
      }
      //catch(Exc_t& exc) {
      catch(Exc_t exc) {
	TMessage m(GledNS::MT_MEE_ConnectionDenied);
	m << exc;
	newsocket->Send(m);
	loop_error = true;
	loop_error_msg = exc;
	break;
      }
      loop_done = true;
      break;
    }

    case GledNS::MT_MEE_Authenticate: {
      ISdebug(0, _eh + "MEE_Authenticate ...");

      UInt_t conn_id;
      *msg >> conn_id;
      try {
	mSunQueen->handle_mee_authentication(conn_id, newsocket);
      }
      catch(Exc_t& exc) {
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
      ISdebug(0, _eh + "got unknown message ... closing connection.");
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
  static const Exc_t _eh("Saturn::finalize_moon_connection ");

  {
    TMessage m(GledNS::MT_MEE_ConnectionGranted);
    lpZGlass_t kings; mGod->CopyList(kings); kings.pop_back();
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

  ISmess(_eh + GForm("(type='%s', name='%s', host='%s').",
		     si->IsA()->GetName(), si->GetName(),
		     si->hSocket->GetInetAddress().GetHostName()));
}

void Saturn::finalize_eye_connection(EyeInfo* ei)
{
  static const Exc_t _eh("Saturn::finalize_eye_connection ");

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
  bAcceptsRays = true;
  mEyeLock.Unlock();

  ISmess(_eh + GForm("(type='%s', name='%s', host='%s').",
		     ei->IsA()->GetName(), ei->GetName(),
		     ei->hSocket->GetInetAddress().GetHostName()));

}


/**************************************************************************/

Int_t Saturn::Manage(TSocket* sock) throw(Exc_t)
{
  static const Exc_t _eh("Saturn::Manage ");

  ISdebug(8, GForm("%sgot sth from\t%s/%s:%d", _eh.Data(),
		   sock->GetInetAddress().GetHostName(),
		   sock->GetInetAddress().GetHostAddress(),
		   sock->GetInetAddress().GetPort()));

  int len, cnt = 0;
  TMessage *m;
  
  while(1) {
  start:
    len = sock->Recv(m);

    if(len == 0) { // closed connection
      ISmess(_eh + "Recv failed w/ code 0, other side closed connection.");
      socket_closed(sock);
      break;
    }
    if(len == -1) { // error
      ISmess(_eh + "Recv failed w/ code -1. Closing connection.");
      socket_closed(sock);
      break;
    }
    // Now all sockets non-blocking ... so the following 'if' is irrelevant.
    // Would need ROOT support for a proper implementation (and several threads).
    if(len == -4) {
      ++cnt;
      cerr << _eh + "Recv failed w/ code -4, cnt="<< cnt <<endl;
      if(cnt > 10) break;
      gSystem->Sleep(100);
      goto start;
    }

    ISdebug(8, GForm("%slen=%d, type=%d", _eh.Data(), len, m->What()));

    switch(m->What()) {

    case kMESS_STRING:
      // ????
      break;
      
    case GledNS::MT_Flare: 
    case GledNS::MT_Beam:
      {
	// Swallows the message 'm', deletes it and sets it to null.
	auto_ptr<ZMIR> mir( new ZMIR(m) );
	mir->ReadRoutingHeader();
	// Could have dedicated thread for listening to the master.
	if(!bSunAbsolute && sock == mSaturnInfo->GetMaster()->hSocket) {
	  mir->Direction = ZMIR::D_Down;
	} else {
	  mir->Direction = ZMIR::D_Up;
	  // These could be handled in separate threads, the first
	  // listening to saturns and the second to eyes.
	  hSock2SocketInfo_i i = mSock2InfoHash.find(sock);
	  if(i == mSock2InfoHash.end()) throw(_eh + "unknown socket");
	  switch(i->second.fWhat) {
	  case SocketInfo::OS_Moon:
	    // ?? Perhaps should check authenticity of caller
	    break;
	  case SocketInfo::OS_Eye:
	    // set caller as eyes don't do that (or even if they do ...)
	    mir->SetCaller( (i->second.get_eye()) );
	    break;
	  default:
	    throw(_eh + "internal inconsistency");
	  } // switch from
	}
	RouteMIR(mir);
	break;
      }
    
    default:
      throw(_eh + "unknown message type");

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
  static const Exc_t _eh("Saturn::handle_mir_result ");

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
    throw(_eh + "unresolved beam result request handle.");
  }
  req->second.mir_rr = res;
  req->second.cond->Lock();
  req->second.cond->Signal();
  req->second.cond->Unlock();
  mBeamReqHandleMutex.Unlock();
}

/**************************************************************************/

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

void Saturn::post_mir(auto_ptr<ZMIR>& mir, ZMirEmittingEntity* caller)
{
  static const Exc_t _eh("Saturn::post_mir ");
  
  ISdebug(8, GForm("%s entered ...", _eh.Data()));

  markup_posted_mir(*mir, caller);

  try {
    RouteMIR(mir);
  }
  catch(Exc_t& exc) {
    ISerr(_eh + "failed: " + exc);
  }
}

void Saturn::shoot_mir(auto_ptr<ZMIR>& mir, ZMirEmittingEntity* caller,
		       bool use_own_thread)
{
  // The auto_ptr<ZMIR> is released and should not be used afterwards.

  markup_posted_mir(*mir, caller);

  if(use_own_thread) {
    mir_router_ti* foo = new mir_router_ti(this, mir.release(), true);
    GThread* bar = new GThread((GThread_foo)(tl_MIR_Router), foo, true);
    bar->Spawn();
  } else {
    mMIRShootingCnd.Lock();
    mMIRShootingQueue.push_back(mir.release());
    mMIRShootingCnd.Broadcast();
    mMIRShootingCnd.Unlock();
  }
}

void Saturn::delayed_shoot_mir(auto_ptr<ZMIR>& mir, ZMirEmittingEntity* caller,
			       GTime at_time)
{
  // The auto_ptr<ZMIR> is released and should not be used afterwards.

  markup_posted_mir(*mir, caller);

  mDelayedMIRShootingCnd.Lock();
  mDelayedMIRShootingQueue.insert(pair<GTime, ZMIR*>(at_time, mir.release()));
  mDelayedMIRShootingCnd.Signal();
  mDelayedMIRShootingCnd.Unlock();
}

/**************************************************************************/

void Saturn::mir_shooter()
{
  // Routes MIRs from the ShootingQueue.
  // Every MIR is deleted after routing.

  static const Exc_t _eh("Saturn::mir_shooter ");

  while(1) {
    mMIRShootingCnd.Lock();
    if(mMIRShootingQueue.empty()) {
      mMIRShootingCnd.Broadcast();
      mMIRShootingCnd.Wait();
    }
    auto_ptr<ZMIR> mir(mMIRShootingQueue.front());
    mMIRShootingQueue.pop_front();
    mMIRShootingCnd.Unlock();
    try {
      mMIRShooterRoutingLock.Lock();
      RouteMIR(mir);
      mMIRShooterRoutingLock.Unlock();
    }
    catch(Exc_t& exc) {
      ISmess(_eh + "caugt exception: " + exc);
    }
  }
}

void Saturn::delayed_mir_shooter()
{
  // Routes MIRs from the ShootingQueue.
  // Every MIR is deleted after routing.

  static const Exc_t _eh("Saturn::delayed_mir_shooter ");

  while(1) {
    mDelayedMIRShootingCnd.Lock();
  begin:
    if(mDelayedMIRShootingQueue.empty()) {
      mDelayedMIRShootingCnd.Wait();
      goto begin;
    }
    mTime2MIR_i i = mDelayedMIRShootingQueue.begin();
    GTime now(GTime::I_Now);
    if(i->first > now) {
      int timeoutp = mDelayedMIRShootingCnd.TimedWaitMS((i->first - now).ToMiliSec());
      if(! timeoutp) goto begin;
    }
    auto_ptr<ZMIR> mir(i->second);
    mDelayedMIRShootingQueue.erase(i);
    mDelayedMIRShootingCnd.Unlock();
    try {
      RouteMIR(mir);
    }
    catch(Exc_t& exc) {
      ISmess(_eh + "caugt exception: " + exc);
    }
  }
}

/**************************************************************************/

void Saturn::PostMIR(auto_ptr<ZMIR>& mir)
{
  // Routes MIR 'mir' in the current thread.
  // Usually called from Operators to publish some result.
  // 'mir' will NOT be harmed; if it is to be re-used, do:
  //  mir.RewindToMIR() prior to posting it again.

  post_mir(mir, 0);
}

void Saturn::PostMIR(ZMIR* mir)
{
  // Routes a MIR in the current thread.
  // Wrapper for auto_ptr function.

  auto_ptr<ZMIR> apmir(mir);
  post_mir(apmir, 0);
  apmir.release();
}

/**************************************************************************/


void Saturn::ShootMIR(auto_ptr<ZMIR>& mir, bool use_own_thread)
{
  // Accepts a MIR and shoots it into the system.
  // The auto_ptr<ZMIR> is released and should not be used afterwards.

  shoot_mir(mir, 0, use_own_thread);
}

void Saturn::ShootMIR(ZMIR* mir, bool use_own_thread)
{
  // Accepts a MIR and shoots it into the system.
  // Wrapper for auto_ptr function.

  auto_ptr<ZMIR> apmir(mir);
  shoot_mir(apmir, 0, use_own_thread);
}

void Saturn::DelayedShootMIR(auto_ptr<ZMIR>& mir, GTime at_time)
{
  // Accepts a MIR and shoots it into the system.
  // The auto_ptr<ZMIR> is released and should not be used afterwards.

  delayed_shoot_mir(mir, 0, at_time);
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

void Saturn::generick_shoot_mir_result(ZMIR& mir, const Text_t* exc, TBuffer* buf)
{
  SaturnInfo* ret_addr = mir.ResultRecipient;
  if(ret_addr == 0) return;

  UChar_t bits = 0;
  if(exc) bits |= ZMIR_RR::B_HasException;
  if(buf) bits |= ZMIR_RR::B_HasResult;

  auto_ptr<ZMIR> result( ret_addr->S_ReceiveBeamResult(mir.ResultReqHandle) );
  *result << bits;
  if(exc) {
    TString s(exc);
    s.Streamer(*result);
  }
  if(buf) {
    result->AppendBuffer(*buf);
  }

  result->SetRecipient(ret_addr);
  result->SetCaller(ret_addr); // The result pretends to be coming come from the caller itself
  result->WriteHeader(); result->RewindToExecHeader();
  mMIRShootingCnd.Lock();
  mMIRShootingQueue.push_back(result.release());
  mMIRShootingCnd.Signal();
  mMIRShootingCnd.Unlock();
  mir.RequiresResult = false;
}

void Saturn::ShootMIRResult(TBuffer& b)
{
  // Makes a result MIR and sends it to the caller Saturn.
  // Should be called from a method called via a MIR (flare OR beam)
  // containing a result request.

  ZMIR* mir = GThread::get_mir();
  if(mir != 0 && mir->RequiresResult) {
    generick_shoot_mir_result(*mir, 0, &b);
  }
}

/**************************************************************************/
// MIR handling
/**************************************************************************/

void Saturn::report_mir_pre_demangling_error(ZMIR& mir, TString error)
{
  static const Exc_t _eh("Saturn::report_mir_pre_demangling_error ");

  ISerr(error);

  mir.Caller = dynamic_cast<ZMirEmittingEntity*>(DemangleID(mir.CallerID));
  if(mir.Caller != 0) {
    auto_ptr<ZMIR> err( mir.Caller->S_Error( error.Data()) );
    err->SetRecipient(mir.Caller->HostingSaturn());
    ShootMIR(err);
    // Here could also add entry into local log.
  } else {
    ISerr(_eh + GForm("could not notify caller id=%u", mir.CallerID));
  }

  // The following is equivalent to ZMIR::DemangleResultRecipient(this).
  if(mir.HasResultReq()) {
    mir.ResultRecipient = dynamic_cast<SaturnInfo*>(DemangleID(mir.ResultRecipientID));
    if(mir.ResultRecipient) {
      generick_shoot_mir_result(mir, error.Data(), 0);
    } else {
      ISerr(_eh + GForm("could not damangle&notify result recipient id=%u", mir.ResultRecipientID));
    }
  }
}

void Saturn::report_mir_post_demangling_error(ZMIR& mir, TString error)
{
  static const Exc_t _eh("Saturn::report_mir_post_demangling_error ");

  ISerr(error);
  
  if(mir.Caller != 0) {
    auto_ptr<ZMIR> err( mir.Caller->S_Error(error.Data()) );
    err->SetRecipient(mir.Caller->HostingSaturn());
    ShootMIR(err);
  } else {
    ISerr(_eh + GForm("could not notify caller id=%u", mir.CallerID));
  }

  if(mir.RequiresResult) {
    generick_shoot_mir_result(mir, error.Data(), 0);
  }
}

//--------------------------------------------------------------------------

void Saturn::RouteMIR(auto_ptr<ZMIR>& mir) throw()
{
  // Decides what to do with a MIR ... and does it.

  static const Exc_t _eh("Saturn::RouteMIR ");

  ISdebug(8, GForm("%s entered ...", _eh.Data()));

  if(mir->AlphaID == 0)
    throw(_eh + "AlphaID is zero!");

  switch(mir->What()) {

  case GledNS::MT_Flare: {
    switch(mir->Direction) {
    case ZMIR::D_Up:
      if(mir->AlphaID < mKing->GetSaturnID()) {
	// This could in principle break if the connection has just been lost.
	// But this is catastrophic anyway.
	ForwardMIR(*mir, mSaturnInfo->GetMaster());
      } else {
	UnfoldMIR(mir);
      }
      break;
    case ZMIR::D_Down:
      UnfoldMIR(mir);
      break;
    default:
      ISerr(_eh + "unknown direction of a flare.");
      break;
    }
    break;
  }

  case GledNS::MT_Beam: {
    try {
      if(mir->RecipientID == 0 || mir->RecipientID == mSaturnInfo->GetSaturnID()) {

	mir->Recipient = mSaturnInfo;
	UnfoldMIR(mir);

      } else {

	mir->Recipient = dynamic_cast<SaturnInfo*>(DemangleID(mir->RecipientID));
	if(mir->Recipient == 0) {
	  throw(_eh + "can't demangle recipient of a beam.");
	}
	
	SaturnInfo* route = mir->Recipient->hRoute;
	if(route == 0) {
	  route = FindRouteTo(mir->Recipient);
	  if(route == 0 || route->hSocket == 0) {
	    throw(_eh + "no route to recipient.");
	  }
	  mir->Recipient->hRoute = route;
	}

	ForwardMIR(*mir, route);

      }
    }
    catch(Exc_t& exc) {
      report_mir_pre_demangling_error(*mir, exc);
    }
    break;
  }

  default:
    ISerr(_eh + "unknown message type. Ignoring.");

  }
}

void Saturn::UnfoldMIR(auto_ptr<ZMIR>& mir) throw()
{
  // Prepares MIR for execution, does checks, proper broadcasting
  // and the execution itself.
  // Message type should be MT_Beam or MT_Flare.

  static const Exc_t _eh("Saturn::UnfoldMIR ");

  ISdebug(8, GForm("%s entered ...", _eh.Data()));

  bool is_shared_space;    // if shared_space and flare => mir should be broadcasted
  bool is_flare = (mir->What() == GledNS::MT_Flare);

  bool is_moon_space = false;
  bool is_sun_space  = false;
  bool is_fire_space = false;

  if(mir->AlphaID < mKing->GetSaturnID())
    {
      is_shared_space = true;
      is_moon_space   = true;
    }
  else if(mir->AlphaID >= mKing->GetSaturnID() &&
	  mir->AlphaID <= mKing->GetMaxID())
    {
      is_shared_space = true;
      is_sun_space    = true;
    }
  else if(mir->AlphaID >= mFireKing->GetSaturnID() &&
	  mir->AlphaID <= mFireKing->GetMaxID())
    {
      is_shared_space = false;
      is_fire_space   = true;
    }
  else
    {
      report_mir_pre_demangling_error(*mir, _eh + "lens ID out of range.");
      return;
    }

  if(mir->HasResultReq()) {
    if((!is_flare || (is_flare && is_sun_space))) {
      try {
	mir->DemangleResultRecipient(this);
      }
      catch(Exc_t& exc) {
	report_mir_pre_demangling_error(*mir, _eh + "could not demangle result recipient.");
	return;
      }
      mir->RequiresResult = true;
    }
  }

  try {
    mir->ReadExecHeader();
    mir->Demangle(this);
  }
  catch(Exc_t& exc) {
    report_mir_pre_demangling_error(*mir, _eh + "context demangling error: " + exc);
    return;
  }

  ZKing* king        = mir->Alpha->GetQueen()->GetKing();
  bool is_king       = (king == mir->Alpha);
  bool is_ruler      = false;
  MIR_Priest* priest = 0;
  lpSaturnInfo_t* reflectors = 0;
  ZGlass* ruler      = 0;

  if(is_king) {
    is_ruler   = true;
    priest     = king;
    reflectors = &mMoons;
    ruler      = king;
  } else {
    ZQueen* queen = mir->Alpha->GetQueen();
    is_ruler   = (mir->Alpha == queen);
    priest     = queen;
    reflectors = &(queen->mReflectors);
    ruler      = queen;
  }

  try {
    if(is_flare) {

      if(is_moon_space) {

	BroadcastMIR(*mir, *reflectors);
	if(mir->ShouldExeDetached()) {
	  ExecDetachedMIR(mir);
	} else {
	  if(is_ruler) { GMutexHolder rulerGMH(mRulingLock); ExecMIR(mir); }
	  else         { ExecMIR(mir); }
	}

      } else {

	{ GMutexHolder blessGMH(ruler->mReadMutex); priest->BlessMIR(*mir); }
	if(mir->ShouldExeDetached()) {
	  if(is_shared_space && mir->IsDetachedExeMultix()) {
	    BroadcastMIR(*mir, *reflectors);
	  }
	  ExecDetachedMIR(mir);
	} else {
	  if(is_ruler) { GMutexHolder rulerGMH(mRulingLock); ExecMIR(mir); }
	  else         { ExecMIR(mir); }
	  if(is_shared_space && mir->SuppressFlareBroadcast == false) {
	    BroadcastMIR(*mir, *reflectors);
	  }
	  if(mir->RequiresResult) generick_shoot_mir_result(*mir, 0, 0);
	}

      }

    } else {

      { GMutexHolder blessGMH(ruler->mReadMutex); priest->BlessMIR(*mir); }
      if(mir->ShouldExeDetached()) {
	ExecDetachedMIR(mir);
      } else {
	if(is_ruler) { GMutexHolder rulerGMH(mRulingLock); ExecMIR(mir); }
	else         { ExecMIR(mir); }
	if(mir->RequiresResult) generick_shoot_mir_result(*mir, 0, 0);
      }

    }
  }
  catch(Exc_t& exc) {      
    report_mir_post_demangling_error(*mir, _eh + "processing failed: " + exc);
    return;
  }
} 

void Saturn::ExecMIR(auto_ptr<ZMIR>& mir, bool lockp)
{
  static const Exc_t _eh("Saturn::ExecMIR ");

  ISdebug(8, GForm("%sentered ...", _eh.Data()));

  if(mir->IsWriting()) mir->RewindToData();

  GledNS::LibSetInfo* lsi = GledNS::FindLibSetInfo(mir->Lid);
  if(lsi == 0)
    throw(_eh + GForm("can't demangle lib ID=%u.", (unsigned int)mir->Lid));

  ZGlass* lens = mir->Alpha;
  ZMirEmittingEntity* ex_mee = GThread::get_owner();
  GThread::set_owner(mir->Caller);
  // This is rarely non-null. But is, e.g. ZQueen::InstantiateWAttach ...
  ZMIR* ex_mir = GThread::get_mir();
  GThread::set_mir(mir.get());
  if(lockp) lens->WriteLock();
  try {
    (lsi->fLME_Foo)(lens, *mir);
  }
  catch(Exc_t& exc) {
    if(lockp) lens->WriteUnlock();
    GThread::set_mir(ex_mir);
    GThread::set_owner(ex_mee);
    throw;
  }
  if(lockp) lens->WriteUnlock();
  GThread::set_mir(ex_mir);
  GThread::set_owner(ex_mee);
}


void Saturn::ExecDetachedMIR(auto_ptr<ZMIR>& mir)
{
  // This function spawns a thread that calls ExecMIR().

  mir_router_ti* foo = new mir_router_ti(this, mir.release(), true);
  // Created joinable, so that it can be canceled and joined.
  GThread* bar = new GThread((GThread_foo)(tl_MIR_DetachedExecutor), foo, false);
  bar->SetEndFoo((GThread_cu_foo)tl_MIR_DetachedCleanUp);
  bar->SetEndArg((void*)foo);
  bar->Spawn();
}

/**************************************************************************/

void Saturn::ForwardMIR(ZMIR& mir, SaturnInfo* route)
{
  // Forwards a MIR to Saturn given by route.

  static const Exc_t _eh("Saturn::ForwardMIR ");

  ISdebug(8, GForm("%s entered ...", _eh.Data()));

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

  if(ret == -1) throw(_eh + "connection just closed.");
}

void Saturn::BroadcastMIR(ZMIR& mir, lpSaturnInfo_t& moons)
{
  // Broadcasts MIR to reflecting Moons specified in socks.

  static const Exc_t _eh("Saturn::BroadcastMIR() ");

  ISdebug(9, GForm("%s entered.", _eh.Data()));

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

  static const Exc_t _eh("Saturn::BroadcastMIR ");

  if(moons.empty()) return;
  bool was_reading = mir.IsReading();
  if(was_reading) mir.Forward();
  mMoonLock.Lock();
  ISdebug(10, _eh + GForm("sending to %d moon(s).", moons.size()));
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
// Ray handling ... viewer notifications.
/**************************************************************************/

void Saturn::ray_emitter()
{
  // Emits Rays from the EmittingQueue.
  // Every MIR is deleted after routing.

  static const Exc_t _eh("Saturn::ray_emitter ");

  while(1) {
    mRayEmittingCnd.Lock();
    if(mRayEmittingQueue.empty())
      mRayEmittingCnd.Wait();

    Ray* ray = mRayEmittingQueue.front();
    mRayEmittingQueue.pop_front();
    mRayEmittingCnd.Unlock();

    mEyeLock.Lock();
    if( ! mEyes.empty()) {
      // cout << _eh << *ray << endl;

      TMessage msg(GledNS::MT_Ray);
      ray->Write(msg);
      ISdebug(10, _eh + GForm("notifying %d eye(s).", mEyes.size()));
      Int_t len = msg.Length() - 4;
      lpEyeInfo_i i = mEyes.begin();
      while(i != mEyes.end()) {
	Int_t ret = (*i)->hSocket->Send(msg);
	if(ret != len) {
	  ISerr(_eh + GForm("sent too little: Eye=%s, len=%3d, ret=%3d.",
			    (*i)->GetName(), len, ret));
	}
	lpEyeInfo_i j = i++;
	if(ret < 0) {
	  wipe_eye(*j, true);
	}
      }
    }
    mEyeLock.Unlock();

    delete ray;
  }
}

void Saturn::Shine(auto_ptr<Ray>& ray)
{
  if(!bAcceptsRays) return;
  mRayEmittingCnd.Lock();
  mRayEmittingQueue.push_back(ray.release());
  mRayEmittingCnd.Signal();
  mRayEmittingCnd.Unlock();
}

void Saturn::DeliverTextMessage(EyeInfo* eye, TextMessage& tm)
{
  if(eye->hSocket == 0) {
    ISerr(GForm("Saturn::DeliverTextMessage got request for non-local eye %s",
		eye->GetName()));
    return;
  }
  TMessage msg(GledNS::MT_TextMessage);
  tm.Streamer(msg);
  mEyeLock.Lock();
  eye->hSocket->Send(msg);
  mEyeLock.Unlock();
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
// Protected & less pleasant
/**************************************************************************/

int Saturn::start_server()
{
  static const Exc_t _eh("Saturn::start_server ");

  int try_count = 0;
open_server_socket:
  int serv_port = mSaturnInfo->GetServerPort();
  mServerSocket = new TServerSocket(serv_port, kTRUE, 4);
  if( !mServerSocket->IsValid() ) {
    if(++try_count <= mSaturnInfo->GetServPortScan()) {
      delete mServerSocket;
      ISwarn(GForm("%sfailed opening server socket at port %d. Trying %d.",
		   _eh.Data(), serv_port, serv_port + 1));
      mSaturnInfo->SetServerPort(serv_port + 1);
      goto open_server_socket;
    }
    int err = 10 - mServerSocket->GetErrorCode();
    ISerr(_eh +"can't create server socket ... dying.");
    return err;
  }
  mSelector.fRead[mServerSocket->GetDescriptor()] = (void*)mServerSocket;
  ISdebug(2, GForm("%sstarting server, port=%d", _eh.Data(),
		   mSaturnInfo->GetServerPort()));
  
  mServerThread = new GThread((GThread_foo)tl_SaturnFdSucker, this, false);
  _server_startup_cond.Lock();
  if( mServerThread->Spawn() ) {
    ISerr(GForm("%scan't create server thread ... dying. errno=%d",
		_eh.Data(), errno));
    return 1;
  }
  _server_startup_cond.Wait();
  _server_startup_cond.Unlock();
  ISdebug(2, _eh + "started server thread.");
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

int Saturn::start_shooters()
{
  const TString _eh("Saturn::start_shooters ");

  mMIRShootingThread = new GThread((GThread_foo)tl_MIR_Shooter, this, false);
  if( mMIRShootingThread->Spawn() ) {
    ISerr(GForm("%scould not create a MirShooter thread ... dying (errno=%d).",
		_eh.Data(), errno));
    return 1;
  }
  ISdebug(2, _eh + "started MirShooter thread.");

  mDelayedMIRShootingThread = new GThread((GThread_foo)tl_Delayed_MIR_Shooter, this, false);
  if( mDelayedMIRShootingThread->Spawn() ) {
    ISerr(GForm("%scould not create a DelayedMirShooter thread (errno=%d).",
		_eh.Data(), errno));
    return 1;
  }
  ISdebug(2, _eh + "started DelayedMirShooter thread.");

  mRayEmittingThread = new GThread((GThread_foo)tl_Ray_Emitter, this, false);
  if( mRayEmittingThread->Spawn() ) {
    ISerr(GForm("%scould not create a RayEmitting thread (errno=%d).",
		_eh.Data(), errno));
    return 1;
  }
  ISdebug(2, _eh + "started RayEmitting thread.");

  return 0;
}

int Saturn::stop_shooters()
{
  if(mMIRShootingThread) {
    mMIRShootingThread->Cancel();
    mMIRShootingThread->Join();
    delete mMIRShootingThread;
    mMIRShootingThread = 0;
  }

  if(mDelayedMIRShootingThread) {
    mDelayedMIRShootingThread->Cancel();
    mDelayedMIRShootingThread->Join();
    delete mDelayedMIRShootingThread;
    mDelayedMIRShootingThread = 0;
  }

  if(mRayEmittingThread) {
    mRayEmittingThread->Cancel();
    mRayEmittingThread->Join();
    delete mRayEmittingThread;
    mRayEmittingThread = 0;
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

  static const Exc_t _eh("Saturn::wipe_moon ");

  ISmess(GForm("%sclosing connection for Moon %s", _eh.Data(), moon->GetName()));
  if(moon->hSocket == 0) {
    // Hmmpsh ... shouldn't really happen ... or just maybe.
    ISmess(_eh + "socket already wiped ... stalling");
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
    ISerr(GForm("%smoon %s not found in hash", _eh.Data(), moon->GetName()));
  }

  lpSaturnInfo_i l = find(mMoons.begin(), mMoons.end(), moon);
  if(l != mMoons.end()) {
    mMoons.erase(l);
  } else {
    ISerr(GForm("%s moon %s not found in list", _eh.Data(), moon->GetName()));
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
    RouteMIR(mir);
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

  if(mEyes.empty())
    bAcceptsRays = false;
  mEyeLock.Unlock();

  if(notify_sunqueen_p) {
    auto_ptr<ZMIR> mir( mSunQueen->S_CremateEye(eye) );
    mir->SetCaller(mSaturnInfo);
    mir->SetRecipient(mSunKing->mSaturnInfo.get());
    mir->WriteHeader(); mir->RewindToExecHeader();
    RouteMIR(mir);
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
  mFireKing = new ZFireKing(mSaturnInfo->mFireKingID + 1, MAX_ID-1, whore_king);
  mFireKing->mSaturnInfo = mSaturnInfo;
  mFireKing->mLightType = ZKing::LT_Fire;
  Enlight(mFireKing, mSaturnInfo->mFireKingID);
  mGod->Add(mFireKing);
  mFireQueen = new ZFireQueen(1024, "FireQueen", "Enchantress of ChaOss");
  mFireKing->Enthrone(mFireQueen);
  mFireQueen->SetMapNoneTo(ZMirFilter::R_Allow);
  mFireQueen->bFollowDeps = true; // !!! This is a wicked trick.
}

void Saturn::arrival_of_kings(TMessage* m)
{
  mSaturnInfo = (SaturnInfo*) GledNS::ReadLensID(*m); // This is ID of *this* Saturn's SaturnInfo
  UInt_t nk; *m >> nk;
  ISmess(GForm("Saturn::arrival_of_kings %d king(s), len=%d",
	       nk, m->BufferSize()));
  for(UInt_t i=0; i<nk; ++i) {
    ZComet c;
    c.Streamer(*m);
    c.RebuildGraph();
    ZKing* k = c.GetKing();
    assert(k!=0);
    Reflect(k);
    k->mLightType = ZKing::LT_Moon;
    lpZGlass_t queens; k->CopyList(queens);
    for(lpZGlass_i j=queens.begin(); j!=queens.end(); ++j) {
      ZQueen* q = dynamic_cast<ZQueen*>(*j);
      assert(q!=0);
      q->mKing = k;
      Reflect(q); q->mQueen = q;
      Reflect(q->mDeps.get()); q->mDeps->mQueen = q;
    }
    k->reference_all();
    for(lpZGlass_i j=queens.begin(); j!=queens.end(); ++j) {
      ZQueen* q = dynamic_cast<ZQueen*>(*j);
      q->reference_all();
      q->mDeps->reference_all();
    }

    if(i == 0) { // This was SunKing and SunQueen is following.
      mSunKing  = k;
      mSunQueen = dynamic_cast<ZSunQueen*>(queens.front());
      assert(mSunQueen!=0);

      //ZComet t; t.Streamer(*m);
      //t.SetExtDemangler(this);
      //t.RebuildGraph();
      //mSunQueen->UnfoldFrom(t);
      mSunQueen->InvokeReflection(*m);

      mSaturnInfo = dynamic_cast<SaturnInfo*>(DemangleID((ID_t)mSaturnInfo));
      assert(mSaturnInfo != 0);
      // Fix mSunInfo and SunKing's SaturnInfo pointer
      mSunInfo = mSunQueen->mSunInfo.get();
      mSunKing->SetSaturnInfo(mSunInfo);
      assert(mSunInfo != 0);
    }
    mGod->Add(k);
  }
}

/**************************************************************************/
