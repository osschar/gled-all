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
#include <Gled/GThread.h>
#include <Stones/ZComet.h>
#include <Gled/GledMirDefs.h>

// Services
#include <Stones/ZHistoManager.h>

#include <TROOT.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TSystem.h>

#include <memory>
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

/**************************************************************************/
// Thread invocators
/**************************************************************************/

namespace {

  GCondition	_server_startup_cond;


  void sh_SaturnFdSucker(int sig) {
    //ISmess(GForm("Saturn/sh_SaturnFdSucker called with signal %d", sig));
  }

  void* tl_SaturnFdSucker(Saturn *s) {
    GThread::SetCancelState(GThread::CS_Enable);
    GThread::SetCancelType(GThread::CT_Deferred);
    signal(SIGUSR2, sh_SaturnFdSucker);
    _server_startup_cond.Lock();
    _server_startup_cond.Signal();
    _server_startup_cond.Unlock();
    // Perhaps should install some exit-foo ... to close sockets ...
    while(1) {
      int ret = s->SockSuck();
      if(ret != 0) {
	ISmess(GForm("Saturn::mServerThread: %s", strerror(errno)));
      }
      GThread::TestCancel();
    }
    return 0;
  }

  void* tl_SaturnAcceptor(void *arg) {
    ((Saturn*)arg)->AcceptWrapper();
    delete GThread::Self();
    GThread::Exit(0);
    return 0;
  }

  struct sat_mir {
    Saturn* sat; ZMIR* mir;
    sat_mir(Saturn* s, ZMIR* c) : sat(s), mir(c) {}
  };

  void* tl_MIRRouter(sat_mir* arg) {
    arg->sat->RouteMIR(*arg->mir);
    delete arg->mir;
    delete arg;
    GThread::Exit(0);
    return 0;
  }

}

/**************************************************************************/

ClassImp(Saturn)

/**************************************************************************/
// Con/De/Structor
/**************************************************************************/

Saturn::Saturn() :
  mIDLock(GMutex::recursive),
  mEyeLock(GMutex::recursive),
  mMoonLock(GMutex::recursive),
  mMasterLock(GMutex::recursive),
  mServerLock(GMutex::recursive),
  mRulingLock(GMutex::recursive),
  mPendingConnection(GMutex::recursive),
  mSaturnInfo(0),
  mQueenLoadNum(0), mQueenLoadCnd(GMutex::recursive),
  mMsg(GledNS::MT_Ray)
{
  bAllowMoons = false;
  mLastID = 0; mStamp = 1;

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

void Saturn::Create(SaturnInfo* si)
{
  mSaturnInfo = si;
  mSaturnInfo->hSocket = 0; // No masters above me

  mSaturnInfo->mKingID = 1;
  fix_fire_king_id(mSaturnInfo);
  create_kings("SunKing", "FireKing");
  mKing->SetTitle("Ruler of what is");
  mFireKing->SetTitle("Ruler of what isn't");

  mSunKing = mKing;

  mSunQueen = new ZSunQueen(666, "SunQueen", "Princess of ChaItOss");
  mSunKing->Enthrone(mSunQueen);
  mSunQueen->CheckIn(mSaturnInfo);
  mSunQueen->SetSunInfo(mSaturnInfo);
  mSaturnInfo->create_lists();

  bSunAbsolute = true;
  if(start_server()) { exit(1); }
}

ID_t Saturn::QueryFFID(SaturnInfo* si)
{
  // Query FirstFreeID

  TSocket s(si->GetMasterName(), si->GetMasterPort());
  // !!!!! check
  int ml; char buf[128];
  ml = s.Recv(buf, 128);
  if(ml == -1) {
    ISerr("Saturn::QueryFFID failed");
    return 0;
  }
  ISmess(GForm("Saturn::QueryFFID: %s", buf));
  s.Send("FFIDQuery");
  TMessage* mp;
  s.Recv(mp);
  ID_t id; *mp >> id;
  delete mp;
  return id;
}

SaturnInfo* Saturn::Connect(SaturnInfo* si)
{
  static string _eh("Saturn::Connect ");

  si->mKingID = QueryFFID(si);
  if(si->mKingID == 0) throw(_eh + "failed FFID query");
  fix_fire_king_id(si);

  TSocket* sock = new TSocket(si->GetMasterName(), si->GetMasterPort());
  // !!!!! check
  {
    int ml; char buf[128];
    ml = sock->Recv(buf, 128);
    if(ml == -1) {
      throw(_eh + "handshake failed");
    }
    ISmess(GForm("Saturn::Connect %s", buf));
    sock->Send("Moon");
  }
  { // Send the desired SatInfo
    TMessage m;
    GledNS::StreamGlass(m, si);
    sock->Send(m);
  }
  {	
    TMessage* m;
    Int_t l = sock->Recv(m);
    if(l <= 0) {
      throw(_eh + "receiving God failed ... dying");
    }
    if(m->What() == GledNS::MT_God) {
      arrival_of_kings(m);
    } else if(m->What() == kMESS_STRING) {
      TString s; *m >> s;
      throw(_eh + "connection refused: " + s.Data());
    } else {
      assert(666==777);
    }
    delete m;
  }

  create_kings("King of the Moon", "FireKing");
  mKing->SetTitle("Ruler of what partially is");
  mFireKing->SetTitle("Ruler of what isn't");

  bSunAbsolute = false;
  mSaturnInfo->hSocket = 0;
  mSaturnInfo->hRoute = 0;
  mSaturnInfo->GetMaster()->hSocket = sock;
  mSaturnInfo->GetMaster()->hRoute  = mSaturnInfo->GetMaster();
  mSelector.fRead[sock->GetDescriptor()] = (void*)sock;
  //sock->SetOption(kNoBlock, 1);

  if(start_server()) exit(1);

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

  // Locking locked-up moon start-up at this point..

  //mServerLock.Lock();
  bAllowMoons = true;
  //mServerLock.Unlock();
}

void Saturn::Shutdown()
{
  // first should stop eventors, dump moons, eyez ...
  ISmess("Saturn::Shutdown stopping server");
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

  // And then fix your little lens
  glass->mSaturnID = id;
  glass->mSaturn = this;
  ISdebug(2,GForm("Saturn::Enlight for %s, id=%u", glass->GetName(), id));
  glass->SetStamps(mStamp);
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
      GThread* at = new GThread(tl_SaturnAcceptor, this, true);
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

void Saturn::AcceptWrapper()
{
  // Exception catching wrapper for Accept().

  mServerLock.Lock();
  try {
    Accept();
  }
  catch(string exc) {
    // Perhaps should report to someone
    cout <<"Saturn::AcceptWrapper caught: "<< exc <<endl;
  }
  mServerLock.Unlock();
}

void Saturn::Accept() throw(string)
{
  // Accepts connection from an Eye/Moon or handles FirstFreeID query.
  // Locks/Suspends all executions upon
  // adding fd to Selector and sending scene to the Moon.
  // Runs in a detached thread (called from tl_SaturnAcceptor) ... 
  // perhaps should timeout.
  // Handles one connection at a time.

  static string _eh("Saturn::Accept ");

  TSocket* newsocket = mServerSocket->Accept();
  if(!newsocket) {
    throw(_eh + "failed.");
  }
  ISdebug(0, GForm("Saturn::Accept Connection from: %s",
		   newsocket->GetInetAddress().GetHostName()));

  // Here should talk to the guy ... 
  newsocket->Send(GForm("This is %s , Saturn version %d",
		       mSaturnInfo->GetName(), Class_Version()));
  // Now check what it is ... Eye | Moon | FFIDQuery
  char s[128];
  int l = newsocket->Recv(s,126);
  s[l+1] = 0;
  if(strcmp("Eye", s) == 0) {
    ////////////////////////////////////////
    // Eye
    ////////////////////////////////////////
    ISdebug(0, "Saturn::Accept Eye ...");

    TMessage *info;
    newsocket->Recv(info); // This is Saturn Info, prefixed by lid,cid
    EyeInfo* ei = dynamic_cast<EyeInfo*>(GledNS::StreamGlass(*info));
    assert(ei!=0);
    delete info;

    // Probably should do checks on many things
    ISdebug(0, GForm("Saturn::Accept EyeName %s", ei->GetName()));

    // Create Yar for execution on the SunQueen
    ZMIR* mir = mSunQueen->S_IncarnateEye(mSaturnInfo);
    GledNS::StreamGlass(*mir->Message, ei);
    mir->SetCaller(mSaturnInfo);
    mir->SetRecipient(mSunQueen->mSunInfo);

    // Route MIR in an alternate thread. Mir is deleted there.
    sat_mir* foo = new sat_mir(this, mir);
    GThread bar((thread_f)(tl_MIRRouter), foo, false);
    mPendingConnection.Lock();
    mPendingEye = 0;
    mPendingSocket = newsocket;
    bar.Spawn();
    ISdebug(0, "Saturn::Accept Waiting for EyeInfo instantiation");
    mPendingConnection.Wait(); // !!!! Should be timed wait, see above.
    bar.Join(); // sat_mir structure should have result I can check
    // Also ... the master must send me a beam, teling me it failed.
    // Otherwise it must be a network problem
    assert(mPendingEye!=0);
    ISdebug(0, "Saturn::Accept EyeInfo received");

    {
      TMessage m(kMESS_ANY);
      m << (UInt_t)this << mPendingEye->GetSaturnID();
      newsocket->Send(m);
    }

    mEyeLock.Lock();
    mSelector.Lock();
    mSelector.fRead[newsocket->GetDescriptor()] = (void*)newsocket;
    mServerThread->Kill(GThread::SigUSR2);
    mSelector.Unlock();
    mSock2InfoHash.insert(pair<TSocket*, SocketInfo>
			  (newsocket, SocketInfo(SocketInfo::OS_Eye,
						 mPendingEye)));
    mEyes.push_back(mPendingEye);
    mEyeLock.Unlock();

    mPendingConnection.Unlock();

  } else if(strcmp("Moon", s) == 0) {
    ////////////////////////////////////////
    // Moon
    ////////////////////////////////////////
    ISdebug(2, "Saturn::Accept Moon ...");

    if(!bAllowMoons) {
      TMessage m(kMESS_STRING);
      TString s("Not accepting moons (yet)");
      m << s;
      newsocket->Send(m);
      ISdebug(0, GForm("Saturn::Accept denying moon Connection from: %s",
		       newsocket->GetInetAddress().GetHostName()));
      newsocket->Close();
      return;
    }

    TMessage *info;
    newsocket->Recv(info); // This is Saturn Info, prefixed by lid,cid
    SaturnInfo* si = dynamic_cast<SaturnInfo*>(GledNS::StreamGlass(*info));
    assert(si!=0);
    delete info;

    // Probably should do checks on many things
    ISdebug(0, GForm("Saturn::Accept MoonName %s", si->GetName()));

    // Create Yar for execution on the SunQueen
    ZMIR* mir = mSunQueen->S_IncarnateMoon(mSaturnInfo);
    GledNS::StreamGlass(*mir->Message, si);
    mir->SetCaller(mSaturnInfo);
    mir->SetRecipient(mSunQueen->mSunInfo);

    // Route MIR in an alternate thread.
    // Should have a mechanism for failure. Like timed wait or could
    // create the thread in non-detached mode and join it ... or sth.
    // No go ... if this is not SunAbsolute the request must make a trip
    // to the Sun.
    // Should work as long as there are no network/streaming errors.
    sat_mir* foo = new sat_mir(this, mir);
    GThread bar((thread_f)(tl_MIRRouter), foo, false);
    mPendingConnection.Lock();
    mPendingMoon = 0;
    mPendingSocket = newsocket;
    bar.Spawn();
    ISdebug(0, "Saturn::Accept Waiting for SaturnInfo instantiation");
    mPendingConnection.Wait(); // !!!! Should be timed wait, see above.
    bar.Join(); // sat_mir structure should have result I can check
    // Also ... the master could send me a beam, teling me it failed.
    assert(mPendingMoon!=0);
    ISdebug(0, "Saturn::Accept SaturnInfo received");
    {
      lpZGlass_t kings; mGod->Copy(kings); kings.pop_back();
      TMessage m(GledNS::MT_God);
      m << mPendingMoon;
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
      newsocket->Send(m);
    }

    mMoonLock.Lock();
    mSelector.Lock();
    mSelector.fRead[newsocket->GetDescriptor()] = (void*)newsocket;
    mServerThread->Kill(GThread::SigUSR2);
    mSelector.Unlock();
    mSock2InfoHash.insert(pair<TSocket*, SocketInfo>
			  (newsocket, SocketInfo(SocketInfo::OS_Moon,
						 mPendingMoon)));
    mMoons.push_back(mPendingMoon);
    mSunQueen->add_reflector(mPendingMoon);
    mPendingMoon->hQueens.insert(mSunQueen);
    mMoonLock.Unlock();

    mPendingConnection.Unlock();

  } else if(strcmp("FFIDQuery", s)==0) {
    ////////////////////////////////////////
    // FirstFreeIDQuery
    ////////////////////////////////////////
    ISdebug(0, "Saturn::Accept FirstFreeIDQuery: reporting/closing");

    ID_t wkid = mFireKing->GetSaturnID();
    TMessage m; m << wkid;
    newsocket->Send(m);
    delete newsocket;
    return;
  } else {
    ////////////////////////////////////////
    // Unknown
    ////////////////////////////////////////
    ISdebug(0, "Saturn::Accept got shit ... closing connection");

    delete newsocket;
    throw(string("Saturn::Accept can't handle stuff like ") + s);
  }
  //newsocket->SetOption(kNoBlock, 1);
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
	ZMIR mir(m); m = 0;
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
// MIR handling
/**************************************************************************/

void Saturn::PostMIR(ZMIR& mir)
{
  // Accepts a MIR for processing.
  // Usually called from Operators to publish some result.
  // Also possible to call it from methods executed via MIRs themselves.
  //
  // If mir is to be re-used, do: mir.RewindToMIR() prior to posting it again.

  static string _eh("Saturn::PostMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  if(mir.Message->What() == GledNS::MT_Flare)
    mir.Direction = ZMIR::D_Up;
  // !!! Should Set Caller based on some Thread/Operator info
  // Now believe the caller.
  // mir.SetCaller(mSaturnInfo);

  try {
    RouteMIR(mir);
  }
  catch (string exc) {
    ISerr(_eh + "failed: " + exc);
  }
}

void Saturn::RouteMIR(ZMIR& mir)
{
  // Decides what to do with a MIR ... and does it.

  static string _eh("Saturn::RouteMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  switch(mir.Message->What()) {

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
	if(route == 0 || route->hSocket == 0)
	  throw(_eh + "no route to recipient");
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

void Saturn::UnfoldMIR(ZMIR& mir)
{
  // Prepares MIR for execution, does checks, proper broadcasting
  // and the execution itself.
  // Message type should be MT_Beam or MT_Flare.

  static string _eh("Saturn::UnfoldMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  try {
    mir.Demangle(this);
  }
  catch(string exc) {
    ISerr(_eh + "Context Demangling exception: " + exc);
    return;
  }

  bool is_shared_space;    // should be broadcasted
  bool is_king  = false;
  bool is_flare = (mir.Message->What() == GledNS::MT_Flare);
  ZKing* king;
  if(mir.AlphaID < mKing->GetSaturnID())
    {
      is_shared_space = true;
      if(mir.Alpha->GetQueen() == 0) {
	king = dynamic_cast<ZKing*>(mir.Alpha);
	if(king != 0) {
	  is_king = true;
	} else {
	  ISerr(_eh + "Alpha wo/ queen (but not king) ... stalling");
	  return;
	}
      } else {
	king = mir.Alpha->GetQueen()->GetKing();
      }
    }
  else if(mir.AlphaID >= mKing->GetSaturnID() &&
	  mir.AlphaID <= mKing->GetMaxID())
    {
      is_shared_space = true;
      king = mKing;
      if(mir.AlphaID == mKing->GetSaturnID()) is_king = true;
    }
  else if(mir.AlphaID >= mFireKing->GetSaturnID() &&
	  mir.AlphaID <= mFireKing->GetMaxID())
    {
      is_shared_space = false;
      king = mFireKing;
      if(mir.AlphaID == mFireKing->GetSaturnID()) is_king = true;
    }
  else
    {
      assert(3==7);
    }

  if(is_king) {
    mRulingLock.Lock();
    try {
      // !!! Checks should be done via mSaturnInfo and/or King itself
      // king->BlessMIR(mir);
      if(is_shared_space && is_flare) BroadcastMIR(mir, mMoons);
      ExecMIR(mir);
    }
    catch (string exc) {
      mRulingLock.Unlock();
      throw;
    }
    mRulingLock.Unlock();
  } else {
    ZQueen* queen = mir.Alpha->GetQueen();
    assert(queen->GetKing() == king);
    bool is_queen = (mir.Alpha == queen);
    if(is_queen) mRulingLock.Lock(); else queen->mExecMutex.Lock();
    try {
      queen->BlessMIR(mir);
      if(is_shared_space && is_flare) {
	// if queen, send to all, else to reflectors.
	lpSaturnInfo_t& moons = is_queen ? mMoons : queen->mReflectors;
	BroadcastMIR(mir, moons);
      }
      ExecMIR(mir);
    }
    catch (string exc) {
      if(is_queen) mRulingLock.Unlock();
      queen->mExecMutex.Unlock();
      EyeInfo* ei = dynamic_cast<EyeInfo*>(mir.Caller);
      if(ei && (is_flare || mir.Alpha->IsSunSpace())) {
	SP_MIR_BEAM(mSaturnInfo, ei->GetMaster(), ei, Error,
		GForm("Saturn::UnfoldMIR exception: %s", exc.c_str()));
      }
      throw;
    }
    if(is_queen) mRulingLock.Unlock();
    queen->mExecMutex.Unlock();
  }
} 

void Saturn::ExecMIR(ZMIR& mir) throw(string)
{
  static string _eh("Saturn::ExecMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  if(mir.Message->IsWriting()) mir.RewindToMIR();

  LID_t libId; *mir.Message >> libId;
  GledNS::LibSetInfo* lsi = GledNS::FindLSI(libId);
  if(lsi == 0)
    throw(string(GForm("Saturn::ExecMIR can't demangle lib ID=%u", (unsigned int)libId)));
  
  mir.Alpha->mExecMutex.Lock();
  mir.Alpha->mMir = &mir;
  try {
    (lsi->fEDFoo)(mir.Alpha, mir.Message);
  }
  catch(string exc) {
    mir.Alpha->mMir = 0;
    mir.Alpha->mExecMutex.Unlock();
    throw;
  }
  mir.Alpha->mMir = 0;
  mir.Alpha->mExecMutex.Unlock();
}

void Saturn::ForwardMIR(ZMIR& mir, SaturnInfo* route)
{
  // Forwards a MIR to Saturn given by route.

  static string _eh("Saturn::ForwardMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  assert(route->hSocket != 0);
  if(mir.Message->IsReading()) mir.Message->Forward();
  bool upwards_p = (route == mSaturnInfo->GetMaster());
  if(upwards_p) mMasterLock.Lock(); else mMoonLock.Lock();

  // cout <<"Saturn::ForwardMIR routing to "<< route->GetName() <<endl;
  Int_t ret = route->hSocket->Send(*mir.Message);
  // cout <<"Saturn::ForwardMIR ret="<< ret <<endl;
  if(ret == -1) socket_closed(route->hSocket);

  if(upwards_p) mMasterLock.Unlock(); else mMoonLock.Unlock();
  if(mir.Message->IsReading()) mir.RewindToMIR();
}

void Saturn::BroadcastMIR(ZMIR& mir, lpSaturnInfo_t& moons)
{
  // Broadcasts MIR to reflecting Moons specified in socks.

  static string _eh("Saturn::BroadcastMIR() ");

  ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  if(moons.empty()) return;
  bool is_reading = mir.Message->IsReading();
  if(is_reading) mir.Message->Forward();
  mMoonLock.Lock();
  ISdebug(9, GForm("Saturn::BroadcastMIR sending to %d moon(s)", moons.size()));
  lpSaturnInfo_i i = moons.begin();
  while(i != moons.end()) {
    if((*i)->hSocket) {
      Int_t ret = (*i)->hSocket->Send(*mir.Message);
      lpSaturnInfo_i j = i++;
      if(ret < 0) {
	// Should be more gracefull !!!!?
	wipe_moon(*j, true);
      }
    }
  }
  mMoonLock.Unlock();
  if(is_reading) mir.RewindToMIR();
}

void Saturn::BroadcastBeamMIR(ZMIR& mir, lpSaturnInfo_t& moons)
{
  // Broadcasts MIR to reflecting Moons specified in socks.

  static string _eh("Saturn::BroadcastMIR() ");

ISdebug(8, GForm("%s entered ...", _eh.c_str()));

  if(moons.empty()) return;
  bool is_reading = mir.Message->IsReading();
  if(is_reading) mir.Message->Forward();
  mMoonLock.Lock();
  ISdebug(9, GForm("Saturn::BroadcastMIR sending to %d moon(s)s", moons.size()));
  lpSaturnInfo_i i = moons.begin();
  while(i != moons.end()) {
    if((*i)->hSocket) {
      mir.SetRecipient(*i);
      Int_t ret = (*i)->hSocket->Send(*mir.Message);
      lpSaturnInfo_i j = i++;
      if(ret < 0) {
	// Should be more gracefull !!!!
	wipe_moon(*j, true);
      }
    }
  }
  mMoonLock.Unlock();
  if(is_reading) mir.RewindToMIR();
}

/**************************************************************************/
// Broadcasting to Eyes
/**************************************************************************/

TimeStamp_t Saturn::Shine(Ray &r)
{
  r.fStamp = ++mStamp; // !!!! Check for mStamp==0 missing
  if(mEyes.empty()) return mStamp;

  mEyeLock.Lock();
  mMsg.Reset();
  r.Streamer( mMsg );
  ISdebug(9, GForm("Saturn::Shine Notifying %d eye(s)", mEyes.size()));
  lpEyeInfo_i i = mEyes.begin();
  while(i != mEyes.end()) {
    Int_t ret = (*i)->hSocket->Send(mMsg);
    lpEyeInfo_i j = i++;
    if(ret < 0) {
      wipe_eye(*j, true);
    }
  }
  mEyeLock.Unlock();

  return mStamp;
}

void Saturn::SingleRay(EyeInfo* eye, Ray& ray) {
  mMsg.Reset();
  if(eye->hSocket == 0) {
    ISerr(GForm("Saturn::SingleRay got request for non-local eye %s",
		eye->GetName()));
  }
  ray.Streamer(mMsg);
  eye->hSocket->Send(mMsg);
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
  
  mServerThread = new GThread((thread_f)tl_SaturnFdSucker, this, false);
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

  if(mServerSocket) {
    mServerSocket->Close();
    delete mServerSocket;
    mServerSocket = 0;
  }

  if(mServerThread) {
    mServerThread->Cancel();
    mServerThread->Join();
    delete mServerThread;
    mServerThread = 0;
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
    mir->SetRecipient(mSunQueen->mSunInfo);
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
    mir->SetRecipient(mSunQueen->mSunInfo);
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
  Enlight(mKing, mSaturnInfo->mKingID);
  mGod->Add(mKing);

  // Fire King
  mFireKing = new ZKing(mSaturnInfo->mFireKingID + 1, MAX_ID-1, whore_king);
  mFireKing->mSaturnInfo = mSaturnInfo;
  mFireKing->bFireKing = true;
  Enlight(mFireKing, mSaturnInfo->mFireKingID);
  mGod->Add(mFireKing);
  mFireQueen = new ZQueen(1024, "FireQueen", "Enchantress of ChaOss");
  mFireKing->Enthrone(mFireQueen);
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
      //t.bWarnOn = false; t.bVerbose = false;
      t.RebuildGraph();
      mSunQueen->UnfoldFrom(t);
      mSaturnInfo = dynamic_cast<SaturnInfo*>(t.FindID((ID_t)mSaturnInfo));
      assert(mSaturnInfo != 0);
      // Fix sun-king's SaturnInfo pointer
      mSunKing->mSaturnInfo = mSunQueen->mSunInfo;
    }
    mGod->Add(k);
  }
}

/**************************************************************************/

void Saturn::new_moon_ready(SaturnInfo* moon)
{
  mPendingConnection.Lock();
  mPendingMoon = moon;
  mPendingConnection.Signal();
  mPendingConnection.Unlock();
}

void Saturn::new_moon_lost() {
  mPendingConnection.Lock();
  mPendingMoon = 0;
  mPendingConnection.Signal();
  mPendingConnection.Unlock();
}

void Saturn::new_eye_ready(EyeInfo* eye)
{
  mPendingConnection.Lock();
  mPendingEye = eye;
  mPendingConnection.Signal();
  mPendingConnection.Unlock();
}

void Saturn::new_eye_lost() {
  mPendingConnection.Lock();
  mPendingEye = 0;
  mPendingConnection.Signal();
  mPendingConnection.Unlock();
}

/**************************************************************************/
