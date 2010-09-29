// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//______________________________________________________________________
// ZSunQueen
//
// The first queen of the Sun Absolute, holding information about all
// Saturns/Eyes connected into the Gled cluster. She also holds lists
// of all identities and group-identities currently used in the sytem
// and handles the MEE authentication procedure. The RSA keys are
// provided by the Gled::theOne singleton.


#include "ZSunQueen.h"
#include <Glasses/ZIdentity.h>
#include <Glasses/ZGroupIdentity.h>
#include <Glasses/ZIdentityFilter.h>
#include <Glasses/ZIdentityListFilter.h>
#include <Glasses/ZFilterAggregator.h>
#include <Glasses/ZMethodTagPreFilter.h>
#include <Glasses/ZMEESelfFilter.h>
#include <Glasses/EyeInfo.h>

#include <Stones/ZMIR.h>
#include <Stones/ZComet.h>
#include <Ephra/Saturn.h>
#include <Gled/GledNS.h>
#include <Gled/Gled.h>
#include <Gled/GKeyRSA.h>

#include "ZSunQueen.c7"

ClassImp(ZSunQueen);

/**************************************************************************/

void ZSunQueen::_init()
{
  mSunInfo = 0;
  mSaturnGuestId = mEyeGuestId = 0;
  bMandatory = true;
}

/**************************************************************************/

ZSunQueen::~ZSunQueen()
{}

/**************************************************************************/

void ZSunQueen::bootstrap()
{
  PARENT_GLASS::bootstrap();

  ZNameMap* auth = new ZNameMap("Auth", "Authentication and Authorization");
  CheckIn(auth); Add(auth);
  {
    ZNameMap* ident = new ZNameMap("Identities", "Identites in the cluster");
    ident->SetElementFID(ZIdentity::FID());
    CheckIn(ident); auth->Add(ident);

    ZNameMap* groups = new ZNameMap("Groups", "GroupIdentites in the cluster");
    groups->SetElementFID(ZGroupIdentity::FID());
    CheckIn(groups); auth->Add(groups);

    ZNameMap* filt = new ZNameMap("Filters", "System-level MIR filters");
    filt->SetElementFID(ZMirFilter::FID());
    CheckIn(filt); auth->Add(filt);

    ZNameMap* idlist = new ZNameMap("IdentityLists", "Standard Identity lists");
    idlist->SetElementFID(ZHashList::FID());
    CheckIn(idlist); auth->Add(idlist);

    {
      ZMEESelfFilter* mee_f = new ZMEESelfFilter("MEESelfFilter");
      CheckIn(mee_f); filt->Add(mee_f);
    }
  }

  /*
  lStr_t ids;
  Gled::theOne->CopyIdentities(ids);
  Gled::theOne->CopyGroupIdentities(ids);
  for(lStr_i i=ids.begin(); i!=ids.end(); ++i) {
    GetOrImportIdentity(i->Data());
  }
  */
}

void ZSunQueen::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
}

/**************************************************************************/

ID_t ZSunQueen::incarnate_moon(SaturnInfo* parent, SaturnInfo* moon)
{
  ID_t id = CheckIn(moon);
  moon->create_lists();
  parent->GetMoons()->Add(moon);
  moon->SetMaster(parent);
  return id;
}

void ZSunQueen::IncarnateMoon(SaturnInfo* parent)
{
  static const Exc_t _eh("ZSunQueen::IncarnateMoon ");

  ZMIR* mir = assert_MIR_presence(_eh);

  SaturnInfo* si = GledNS::StreamLensByGlass<SaturnInfo*>(*mir);
  if(si == 0) {
    throw(_eh + "did not receive a lens of glass SaturnInfo");
  }
  incarnate_moon(parent, si);
}

ID_t ZSunQueen::incarnate_eye(SaturnInfo* parent, EyeInfo* eye)
{
  ID_t id = CheckIn(eye);
  parent->GetEyes()->Add(eye);
  eye->SetMaster(parent);
  return id;
}

void ZSunQueen::IncarnateEye(SaturnInfo* parent)
{
  static const Exc_t _eh("ZSunQueen::IncarnateEye ");

  ZMIR* mir = assert_MIR_presence(_eh);

  EyeInfo* ei = GledNS::StreamLensByGlass<EyeInfo*>(*mir);
  if(ei == 0) {
    throw(_eh + "did not receive a lens of glass EyeInfo");
  }
  incarnate_eye(parent, ei);
}

void ZSunQueen::CremateMoon(SaturnInfo* moon)
{
  // Removes moon (and all its satellites) from celestial bodies.
  // If sent as MT_Beam interprets it as request to shut down
  // connection to the moon with notification sent to SunQueen.

  static TString _eh("ZSunQueen::CremateMoon ");
  ZMIR* mir = ZGlass::assert_MIR_presence(_eh);

  if(moon->GetMaster() == mSaturn->GetSaturnInfo()) {
    if(moon->hSocket != 0) {
      ISmess(_eh + "socket still present ... closing it");
      if(mir->What() == GledNS::MT_Beam) {
	mSaturn->wipe_moon(moon, true);
	return;
      } else {
	mSaturn->wipe_moon(moon, false);
      }
    }
  }

  if(mSaturn->GetSunAbsolute()) {
    mir->ClearRecipient();
    mir->SetCaller(mSunInfo.get());
    mSaturn->BroadcastMIR(*mir, mSaturn->mMoons);
  }

  detach_all_identities(moon);
  mOrphans->Add(moon);
  moon->GetMaster()->GetMoons()->Remove(moon);
  moon->SetMaster((SaturnInfo*)0); moon->hRoute = 0;
}

void ZSunQueen::CremateEye(EyeInfo* eye)
{
  static const Exc_t _eh("ZSunQueen::CremateEye ");

  ZMIR* mir = ZGlass::assert_MIR_presence(_eh);

  if(eye->GetMaster() == mSaturn->GetSaturnInfo()) {
    if(eye->hSocket != 0) {
      ISmess(_eh + "socket still present ... closing it.");
      if(mir->What() == GledNS::MT_Beam) {
	mSaturn->wipe_eye(eye, true);
	return;
      } else {
	mSaturn->wipe_eye(eye, false);
      }
    }
  }

  if(mSaturn->GetSunAbsolute()) {
    mir->ClearRecipient();
    mir->SetCaller(mSunInfo.get());
    mSaturn->BroadcastMIR(*mir, mSaturn->mMoons);
  }

  detach_all_identities(eye);
  mOrphans->Add(eye);
  eye->GetMaster()->GetEyes()->Remove(eye);
  eye->SetMaster((SaturnInfo*)0);
}

/**************************************************************************/
// Handling of new connections
/**************************************************************************/


void ZSunQueen::handle_mee_connection(ZMirEmittingEntity* mee, TSocket* socket)
{
  // Called by Saturn upon receiving a MEE connection request.
  // Handles most of the connection stuff on the Saturn side, in particular
  // all communication with Sun Absolute.

  static const Exc_t _eh("ZSunQueen::handle_mee_connection ");

  ZMIR* mir_ptr;
  if (GledNS::IsA(mee, SaturnInfo::FID()))
  {
    if (mSaturn->GetAllowMoons() == false)
    {
      printf("type = %s\n", typeid(_eh + "frek").name());
      throw _eh + "Saturn is not accepting moon connections.";
    }
    mir_ptr = S_initiate_saturn_connection();
  }
  else if (GledNS::IsA(mee, EyeInfo::FID()))
  {
    mir_ptr = S_initiate_eye_connection();
  }
  else
  {
    throw _eh + "unknown type of mee.";
  }
  auto_ptr<ZMIR> mir(mir_ptr);
  GledNS::StreamLens(*mir, mee);
  mir->SetRecipient(mSunInfo.get());
  delete mee;

  ISdebug(0, _eh + "Waiting for MirEmittingEntity instantiation.");
  auto_ptr<ZMIR_RR> ret ( mSaturn->ShootMIRWaitResult(mir) );
  if (!ret->BeamResult_OK())
    throw Exc_t(ret->GenError());

  UChar_t crr_buf; *ret >> crr_buf;
  ConnReqResult_e crr = (ConnReqResult_e)crr_buf;

  if (crr == CRR_Denied)
    throw _eh + "connection denied.";

  ID_t mee_id;

  if (crr == CRR_ReqAuth)
  {
    UInt_t conn_id; *ret >> conn_id;
    GCondition auth_cond;
    NCSData* ncsd = new NCSData(&auth_cond);
    mNCSlaveData.insert(conn_id, ncsd);
    auth_cond.Lock();

    TMessage m(GledNS::MT_MEE_AuthRequested);
    mSunInfo->mHostName.Streamer(m);
    m << mSunInfo->mServerPort;
    m << conn_id;
    socket->Send(m);

    auth_cond.Wait();
    auth_cond.Unlock();
    ConnReqResult_e state = ncsd->fState;
    mee_id = ncsd->fMeeID;
    mNCSlaveData.remove(conn_id);
    if (state != CRR_OK)
      throw _eh + "permission denied.";
  }
  else if (crr == CRR_OK)
  {
    *ret >> mee_id;
  }
  else
  {
    throw _eh + "unknown reponse from master.";
  }

  mee = dynamic_cast<ZMirEmittingEntity*>(mSaturn->DemangleID(mee_id));
  if (mee == 0)
    throw _eh + "received CRR_OK but can't demangle MirEmittingEntity.";

  if (GledNS::IsA(mee, SaturnInfo::FID()))
  {
    SaturnInfo* si = dynamic_cast<SaturnInfo*>(mee);
    si->hSocket = socket;
    mSaturn->finalize_moon_connection(si);
  }
  else if (GledNS::IsA(mee, EyeInfo::FID()))
  {
    EyeInfo* ei = dynamic_cast<EyeInfo*>(mee);
    ei->hSocket = socket;
    mSaturn->finalize_eye_connection(ei);
  }
  else
  {
    throw _eh + "unknow type of MEE.";
  }

  ISdebug(0, _eh + "MirEmittingEntity received and finalised.");

}

/**************************************************************************/

void ZSunQueen::initiate_saturn_connection()
{
  // Wrapper for initiate_mee_connection() with
  // SunQueen::SaturnConnections tag.

  initiate_mee_connection();
}

void ZSunQueen::initiate_eye_connection()
{
  // Wrapper for initiate_mee_connection() with
  // SunQueen::EyeConnections tag.

  initiate_mee_connection();
}

void ZSunQueen::initiate_mee_connection()
{
  // Sent as a beam with result request from a Saturn that receives
  // a request for connection of a new Saturn to the Sun Absolute.
  // The desired MirEmittingEntity must be attached to the MIR.

  static const Exc_t _eh("ZSunQueen::initiate_mee_connection ");

  ZMIR* mir = assert_MIR_presence(_eh, ZGlass::MC_IsBeam | ZGlass::MC_HasResultReq);

  SaturnInfo* req = dynamic_cast<SaturnInfo*>(mir->fCaller);
  if(req == 0)
    throw(_eh + "caller not a Saturn");

  ZMirEmittingEntity* mee =
    dynamic_cast<ZMirEmittingEntity*>(GledNS::StreamLens(*mir));
  if(mee == 0)
    throw(_eh + "did not receive a lens of glass ZMirEmittingEntity");

  bool use_auth = mSunInfo->GetUseAuth();
  if(use_auth && mee->mLogin == "guest") {
    if(GledNS::IsA(mee, SaturnInfo::FID())) {
      if(mSaturnGuestId != 0) {
	mee->mLogin = mSaturnGuestId->mName;
      } else {
	throw(_eh + "not accepting guest Saturns");
      }
    }
    else if(GledNS::IsA(mee, EyeInfo::FID())) {
      if(mEyeGuestId != 0) {
	mee->mLogin = mEyeGuestId->mName;
      } else {
	throw(_eh + "not accepting guest Eyes");
      }
    }
    else {
      throw(_eh + "unknown type of MEE");
    }
    use_auth = false;
  }

  if(use_auth) {
    try {
      Gled::theOne->GetPubKeyFile(mee->mLogin);
    }
    catch(Exc_t& exc) {
      throw(_eh + "unknown identity (" + exc + ")"); // !!!! should deny below
    }
  }

  NCMData* ncmd = new NCMData(mee, req);
  UInt_t cid = mNCMasterData.insert(ncmd);

  TBufferFile ret(TBuffer::kWrite);
  if(use_auth) {
    ret << (UChar_t)CRR_ReqAuth;
    ret << cid;
  } else {
    ID_t mee_id = incarnate_mee(cid);
    ret << (UChar_t)CRR_OK;
    ret << mee_id;
    mNCMasterData.remove(cid);
  }
  mSaturn->ShootMIRResult(ret);
}

void ZSunQueen::handle_mee_authentication(UInt_t conn_id, TSocket* socket)
{
  static const Exc_t _eh("ZSunQueen::handle_mee_authentication ");
  TString err;

  NCMData* ncmd = mNCMasterData.retrieve(conn_id);
  if(ncmd == 0) {
    throw(_eh + "unknown conn_id");
  }
  GKeyRSA sun, mee;
  try {
    mee.ReadPubKey(Gled::theOne->GetPubKeyFile(ncmd->fNewMEE->mLogin));
    sun.ReadPrivKey(Gled::theOne->GetPrivKeyFile(mSunInfo->mLogin));
    // !!!! get saturn identity from somewhere else
    // !!!! should assert key of mee equal or longer from sun key
    mee.GenerateSecret();
  }
  catch(Exc_t& exc) {
    err = "key handling failed";
    goto auth_failed;
  }
  {
    TMessage m(GledNS::MT_Auth_Challenge);
    mee.SendSecret(m);
    sun.StreamPubKey(m);
    socket->Send(m);
  }
  {
    TMessage* m;
    Int_t     l;
    l = socket->Recv(m);
    if(l<=0 || m->What() != GledNS::MT_Auth_ChallengeResponse) {
      delete m;
      err = "failure receiving Auth_ChallengeResponse";
      goto auth_failed;
    }
    sun.ReceiveSecret(*m);
    if(mee.MatchSecrets(sun) == false) {
      delete m;
      err = "authentication failed";
      goto auth_failed;
    }
  }

  {
    ID_t mee_id = incarnate_mee(conn_id);
    auto_ptr<ZMIR> mir( S_accept_mee_connection(conn_id, mee_id) );
    mir->SetRecipient(ncmd->fRequestor);
    mSaturn->ShootMIR(mir);
    mNCMasterData.remove(conn_id);
    return;
  }

 auth_failed: {
    auto_ptr<ZMIR> mir( S_deny_mee_connection(conn_id) );
    mir->SetRecipient(ncmd->fRequestor);
    mSaturn->ShootMIR(mir);
    mNCMasterData.remove(conn_id);
    throw(_eh + err);
  }
}

/**************************************************************************/

void ZSunQueen::accept_mee_connection(UInt_t conn_id, ID_t mee_id)
{
  static const Exc_t _eh("ZSunQueen::accept_mee_connection ");

  NCSData* ncsd = mNCSlaveData.retrieve(conn_id);
  if(ncsd == 0) {
    throw _eh + "couldn't retrieve conn key";
  }
  ncsd->fCond->Lock();
  ncsd->fState = CRR_OK;
  ncsd->fMeeID = mee_id;
  ncsd->fCond->Signal();
  ncsd->fCond->Unlock();
}

void ZSunQueen::deny_mee_connection(UInt_t conn_id)
{
  static const Exc_t _eh("ZSunQueen::drop_mee_connection ");

  NCSData* ncsd = mNCSlaveData.retrieve(conn_id);
  if(ncsd == 0) {
    throw _eh + "couldn't retrieve conn key";
  }
  ncsd->fCond->Lock();
  ncsd->fState = CRR_Denied;
  ncsd->fCond->Signal();
  ncsd->fCond->Unlock();
}

/**************************************************************************/

ID_t ZSunQueen::incarnate_mee(UInt_t conn_id)
{
  // Called on SunAbsolute to actually insert given MEE into the hierarchy.
  // Creates a flare and shoots id with set-uid to SunAbsolute.
  // Also calls attach_primary_identity().

  static const Exc_t _eh("ZSunQueen::incarnate_mee ");

  NCMData* ncd = mNCMasterData.retrieve(conn_id);
  if(ncd == 0)
    throw(_eh + "ConnectionData not available");

  ID_t id;
  ZMIR* mirp = 0;
  TBufferFile streamed_mee(TBuffer::kWrite);

  if(GledNS::IsA(ncd->fNewMEE, SaturnInfo::FID())) {
    SaturnInfo* si = (SaturnInfo*)ncd->fNewMEE;
    si->bUseAuth = mSunInfo->bUseAuth;
    GledNS::StreamLens(streamed_mee, si);

    id   = incarnate_moon(ncd->fRequestor, si);
    mirp = S_IncarnateMoon(ncd->fRequestor);
  }
  else if(GledNS::IsA(ncd->fNewMEE, EyeInfo::FID())) {
    EyeInfo* ei = (EyeInfo*)ncd->fNewMEE;
    GledNS::StreamLens(streamed_mee, ei);

    id   = incarnate_eye(ncd->fRequestor, ei);
    mirp = S_IncarnateEye(ncd->fRequestor);
  } else {
    throw(_eh + "unknow type of MEE");
  }

  auto_ptr<ZMIR> mir(mirp);
  mir->AppendBuffer(streamed_mee);
  mSaturn->markup_posted_mir(*mir);
  mSaturn->BroadcastMIR(*mir, mReflectors);

  attach_primary_identity(ncd->fNewMEE);

  return id;
}

/**************************************************************************/
/**************************************************************************/

ZIdentity* ZSunQueen::GetOrImportIdentity(const char* ident)
{
  static const Exc_t _eh("ZSunQueen::GetOrImportIdentity ");

  ZIdentity* identity = 0;
  const char* path;
  FID_t fid;

  if(ident[0] == '@') {
    path = "Auth/Groups";
    fid = ZGroupIdentity::FID();
  } else {
    path = "Auth/Identities";
    fid = ZIdentity::FID();
  }

  ZNameMap* id_dir = dynamic_cast<ZNameMap*>(FindLensByPath(path));
  if(id_dir == 0)
    throw _eh + "can not retrieve the identity directory";

  identity = dynamic_cast<ZIdentity*>(id_dir->FindLensByPath(ident));
  if(identity != 0)
    return identity;

  identity = (ZIdentity*)
    CALL_AND_BROADCAST(this, instantiate, fid,
		       ident,
		       GForm("Base identity for %s", ident));
  CALL_AND_BROADCAST(id_dir, Add, identity);

  ZIdentityFilter* id_filter = (ZIdentityFilter*)
    CALL_AND_BROADCAST(this, instantiate, ZIdentityFilter::FID(),
		       GForm("Allow %s", ident),
		       GForm("Identity filter for %s", ident));

  // Cross-link the two.
  CALL_AND_BROADCAST(identity, SetAllowThis, id_filter);
  CALL_AND_BROADCAST(id_filter, SetIdentity, identity);

  return identity;
}

/**************************************************************************/

void ZSunQueen::AttachIdentity(ZIdentity* id)
{
  static TString _eh("ZSunQueen::AttachIdentity ");

  ZMIR* mir = assert_MIR_presence(_eh);

  if(id == 0)
    throw(_eh + "null identity");
  if(mir->fCaller->HasIdentity(id))
    throw(_eh + GForm("%s already has %s", mir->fCaller->GetName(), id->GetName()));

  ZGroupIdentity* gid = dynamic_cast<ZGroupIdentity*>(id);
  if(gid == 0)
    throw(_eh + "for now only handles group identities");

  lpZGlass_t mee_ids;
  mee_ids.push_back(mir->fCaller->mPrimaryIdentity.get());
  mir->fCaller->mActiveIdentities->CopyList(mee_ids);
  for(lpZGlass_i i=mee_ids.begin(); i!=mee_ids.end(); ++i) {

    if(Gled::theOne->IsIdentityInGroup((*i)->GetName(), id->GetName())) {

      CALL_AND_BROADCAST(mir->fCaller->mActiveIdentities, Add, id);
      CALL_AND_BROADCAST(id->mActiveMEEs, Add, mir->fCaller);

      mir->fSuppressFlareBroadcast = true;
      return;
    }

  }
  throw(_eh + "insufficient karma");
}

void ZSunQueen::DetachIdentity(ZIdentity* id)
{
  static TString _eh("ZSunQueen::DetachIdentity ");

  ZMIR* mir = assert_MIR_presence(_eh);
  if(id == 0)
    throw(_eh + "null identity");
  if(!mir->fCaller->HasIdentity(id))
    throw(_eh + GForm("%s does not posses identity %s", mir->fCaller->GetName(), id->GetName()));
  if(mir->fCaller->mPrimaryIdentity == id)
    throw(_eh + GForm("primary identity %s can not be detached from %s",id->GetName(), mir->fCaller->GetName()));

  ZGroupIdentity* gid = dynamic_cast<ZGroupIdentity*>(id);
  if(gid == 0)
    throw(_eh + "for now only handles group identities");

  CALL_AND_BROADCAST(mir->fCaller->mActiveIdentities, Remove, id);
  CALL_AND_BROADCAST(id->mActiveMEEs, Remove, mir->fCaller);

  mir->fSuppressFlareBroadcast = true;
}

/**************************************************************************/

void ZSunQueen::attach_primary_identity(ZMirEmittingEntity* mee)
{
  static const Exc_t _eh("ZSunQueen::attach_primary_identity ");

  ZIdentity* identity = GetOrImportIdentity(mee->GetLogin());
  CALL_AND_BROADCAST(mee, SetPrimaryIdentity, identity);
  CALL_AND_BROADCAST(identity->GetActiveMEEs(), Add, mee);

  ZMirFilter* def_filter = dynamic_cast<ZMirFilter*>(
			     FindLensByPath("Auth/Filters/MEESelfFilter"));
  CALL_AND_BROADCAST(mee, SetGuard, def_filter);
}

void ZSunQueen::detach_all_identities(ZMirEmittingEntity* mee)
{
  static const Exc_t _eh("ZSunQueen::detach_all_identities ");

  ZIdentity* id;
  mee->WriteLock();
  if((id = mee->mPrimaryIdentity.get()) != 0) {
    mee->SetPrimaryIdentity(0);
    id->mActiveMEEs->Remove(mee);
  }
  {
    Stepper<ZIdentity> s(*mee->mActiveIdentities);
    while(s.step())
      s->mActiveMEEs->Remove(mee);
    mee->mActiveIdentities->ClearList();
  }

  SaturnInfo* si = dynamic_cast<SaturnInfo*>(mee);
  if(si != 0) {
    lpZGlass_t l;
    si->GetMoons()->CopyList(l);
    si->GetEyes()->CopyList(l);
    for(lpZGlass_i i=l.begin(); i!=l.end(); ++i) {
      detach_all_identities((ZMirEmittingEntity*)*i);
    }
  }
  mee->WriteUnlock();
}

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
/**************************************************************************/

void ZSunQueen::HandleClientSideAuthentication(TSocket* socket, UInt_t conn_id,
					       TString& identity)
{
  static const Exc_t _eh("ZSunQueen::HandleClientSideAuthentication ");

  {
    TMessage m(GledNS::MT_MEE_Authenticate);
    m << conn_id;
    socket->Send(m);
  }
  {
    TMessage* m;
    Int_t     l;
    l = socket->Recv(m);
    if(l<=0 || m->What() != GledNS::MT_Auth_Challenge) {
      delete m;
      throw(_eh + "failure receiving Auth_Challenge");
    }
    GKeyRSA key;
    try {
      key.ReadPrivKey(Gled::theOne->GetPrivKeyFile(identity));
    }
    catch(Exc_t& exc) {
      delete m;
      throw(_eh + "error reading private key (" + exc + ")");
    }
    key.ReceiveSecret(*m);
    key.StreamPubKey(*m);

    TMessage r(GledNS::MT_Auth_ChallengeResponse);
    key.SendSecret(r);
    socket->Send(r);
  }
}
