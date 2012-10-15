// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XrdServer.h"
#include "XrdUser.h"
#include "XrdFile.h"
#include "XrdServer.c7"

// XrdServer

//______________________________________________________________________________
//
// Represents a running xrootd server instance.
// Stores current sessions, represented by glass XrdUser, as list
// elements (lookup by name). Map dict_id -> XrdUser is also kept.
// Once a user disconnects, it is moved to owned list mPrevUsers.
//
// Map dict_id -> XrdFile is also stored here.

ClassImp(XrdServer);

//==============================================================================

void XrdServer::_init()
{
  mLastSrvIdTime.SetNever();
  mAvgSrvIdDelta = -1;
  mPacketCount = mSeqIdFailCount = 0;
  ResetSrvSeq();
}

XrdServer::XrdServer(const TString& n, const TString& t) :
  ZNameMap(n, t)
{
  _init();
}

XrdServer::XrdServer(const TString& n, const TString& t, const TString& h, const TString& d, const GTime& st) :
  ZNameMap(n, t),
  mHost(h), mDomain(d), mStartTime(st)
{
  _init();
}

XrdServer::~XrdServer()
{}

//------------------------------------------------------------------------------

void XrdServer::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if (mPrevUsers == 0)
  {
    assign_link<ZList>(mPrevUsers, FID(), "PrevUsers",
                       GForm("Previous users of %s", GetName()));
    mPrevUsers->SetElementFID(XrdUser::FID());
    mPrevUsers->SetMIRActive(false);
  }
}

//------------------------------------------------------------------------------

TString XrdServer::GetFqhn() const
{
  return mHost + "." + mDomain;
}

//==============================================================================

void XrdServer::UpdateSrvIdTime(const GTime& t)
{
  if ( ! mLastSrvIdTime.IsNever())
  {
    Int_t dt = TMath::Max(10, TMath::Nint((t - mLastSrvIdTime).ToDouble()));
    if (mAvgSrvIdDelta <= 0)
    {
      mAvgSrvIdDelta = dt;
    }
    else if (dt < mAvgSrvIdDelta)
    {
      mAvgSrvIdDelta = TMath::Nint(0.9*mAvgSrvIdDelta + 0.1*dt);
    }
  }
  mLastSrvIdTime = t;
  Stamp(FID());
}

//==============================================================================

void XrdServer::IncPacketCount()
{
  if (++mPacketCount % 100 == 0)
    Stamp(FID());
}

void XrdServer::IncSeqIdFailCount()
{
  ++mSeqIdFailCount;
  Stamp(FID());
}

//==============================================================================

Bool_t XrdServer::ExistsUserDictId(UInt_t dict_id)
{
  GMutexHolder _lck(mUserMapMutex);
  return mUserMap.find(dict_id) != mUserMap.end();
}

void XrdServer::AddUser(XrdUser* user, UInt_t dict_id)
{
  static const Exc_t _eh("XrdServer::AddUser ");

  GMutexHolder _lck(mUserMapMutex);

  if (mUserMap.find(dict_id) != mUserMap.end())
    throw _eh + "dict_id exists already.";

  Add(user);

  mUserMap.insert(make_pair(dict_id, user));
  user->m_dict_id = dict_id;
}

void XrdServer::DisconnectUser(XrdUser* user)
{
  static const Exc_t _eh("XrdServer::DisconnectUser ");

  {
    GMutexHolder _lck(mUserMapMutex);
    mDict2User_i i = mUserMap.find(user->m_dict_id);
    if (i == mUserMap.end())
      throw _eh + "dict_id does not exist for user '" + user->GetName() + "'.";
    mUserMap.erase(i);

    mPrevUserMap.insert(make_pair(user->m_dict_id, user));
  }

  mPrevUsers->PushBack(user);
  RemoveAll(user);
}

void XrdServer::RemovePrevUser(XrdUser* user)
{
  {
    GMutexHolder _lck(mUserMapMutex);
    mPrevUserMap.erase(user->m_dict_id);
  }
  mPrevUsers->RemoveAll(user);
}

Int_t XrdServer::RemovePrevUsersOlderThan(const GTime& cut_time)
{
  // Removes previous users older than given cut_time.

  // We know old users are pushed-back sequentially, so we can check queue
  // front only.

  XrdUser *u;
  Int_t    uid;
  Int_t    n_wiped = 0;

  while (true)
  {
    {
      GMutexHolder _lck(mPrevUsers->RefListMutex());

      u = (XrdUser*) mPrevUsers->FrontElement();
      if (! u || u->RefDisconnectTime() > cut_time)
        break;

      uid = u->m_dict_id;
      mPrevUsers->PopFront();
    }
    {
      GMutexHolder _lck(mUserMapMutex);
      mPrevUserMap.erase(uid);
    }
    if (! u->CheckBit(kDyingBit))
    {
      mQueen->RemoveLens(u);
    }
    ++n_wiped;
  }

  return n_wiped;
}

void XrdServer::ClearPrevUserMap()
{
  GMutexHolder _lck(mUserMapMutex);
  mPrevUserMap.clear();
}

XrdUser* XrdServer::FindUser(const TString& name)
{
  return dynamic_cast<XrdUser*>(GetElementByName(name));
}

XrdUser* XrdServer::FindUser(UInt_t dict_id)
{
  GMutexHolder _lck(mUserMapMutex);
  mDict2User_i i = mUserMap.find(dict_id);
  return (i != mUserMap.end()) ? i->second : 0;
}

XrdUser* XrdServer::FindUserOrPrevUser(UInt_t dict_id)
{
  mDict2User_i i;

  GMutexHolder _lck(mUserMapMutex);

  i = mUserMap.find(dict_id);
  if (i != mUserMap.end()) return i->second;

  i = mPrevUserMap.find(dict_id);
  if (i != mPrevUserMap.end()) return i->second;

  return 0;
}

//------------------------------------------------------------------------------

Bool_t XrdServer::ExistsFileDictId(UInt_t dict_id)
{
  GMutexHolder _lck(mFileMapMutex);
  return mFileMap.find(dict_id) != mFileMap.end();
}

void XrdServer::AddFile(XrdFile* file, UInt_t dict_id)
{
  static const Exc_t _eh("XrdServer::AddFile ");

  GMutexHolder _lck(mFileMapMutex);

  if (mFileMap.find(dict_id) != mFileMap.end())
    throw _eh + "dict_id exists already.";

  mFileMap.insert(make_pair(dict_id, file));
  file->m_dict_id = dict_id;
}

void XrdServer::RemoveFile(XrdFile* file)
{
  static const Exc_t _eh("XrdServer::RemoveFile ");

  GMutexHolder _lck(mFileMapMutex);

  mDict2File_i i = mFileMap.find(file->m_dict_id);
  if (i == mFileMap.end())
    throw _eh + "dict_id not registered for file '" + file->GetName() + "'.";
  if (i->second != file)
    throw _eh + "stored file '" + file->GetName() +
                "' and file passed as argument '" + i->second->GetName() +
                "' do not match.";

  mFileMap.erase(i);
}

XrdFile* XrdServer::FindFile(UInt_t dict_id)
{
  GMutexHolder _lck(mFileMapMutex);

  mDict2File_i i = mFileMap.find(dict_id);
  return (i != mFileMap.end()) ? i->second : 0;
}
