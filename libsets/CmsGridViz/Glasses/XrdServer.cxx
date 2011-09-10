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
{}

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
    mPrevUsers = new ZList("PrevUsers", GForm("Previous users of %s", GetName()));
    mPrevUsers->SetElementFID(XrdUser::FID());
    mPrevUsers->SetMIRActive(false);
    mQueen->CheckIn(mPrevUsers.get());
  }
}

//==============================================================================

Bool_t XrdServer::ExistsUserDictId(Int_t dict_id)
{
  return mUserMap.find(dict_id) != mUserMap.end();
}

void XrdServer::AddUser(XrdUser* user, Int_t dict_id)
{
  static const Exc_t _eh("XrdServer::AddUser ");

  if (mUserMap.find(dict_id) != mUserMap.end())
    throw _eh + "dict_id exists already.";

  Add(user);
  mUserMap.insert(make_pair(dict_id, user));
}

void XrdServer::DisconnectUser(XrdUser* user, Int_t dict_id)
{
  static const Exc_t _eh("XrdServer::DisconnectUser ");

  if (mUserMap.find(dict_id) == mUserMap.end())
    throw _eh + "dict_id does not exist.";

  mPrevUsers->Add(user);
  RemoveAll(user);
}

void XrdServer::RemoveUser(XrdUser* user)
{
  mPrevUsers->RemoveAll(user);
}

XrdUser* XrdServer::FindUser(const TString& name)
{
  return dynamic_cast<XrdUser*>(GetElementByName(name));
}

XrdUser* XrdServer::FindUser(Int_t dict_id)
{
  mDict2User_i i = mUserMap.find(dict_id);
  return (i != mUserMap.end()) ? i->second : 0;
}

//------------------------------------------------------------------------------

Bool_t XrdServer::ExistsFileDictId(Int_t dict_id)
{
  return mFileMap.find(dict_id) != mFileMap.end();
}

void XrdServer::AddFile(XrdFile* file, Int_t dict_id)
{
  static const Exc_t _eh("XrdServer::AddFile ");

  if (mFileMap.find(dict_id) != mFileMap.end())
    throw _eh + "dict_id exists already.";

  mFileMap.insert(make_pair(dict_id, file));
}

void XrdServer::RemoveFile(XrdFile* file, Int_t dict_id)
{
  static const Exc_t _eh("XrdServer::RemoveFile ");

  mDict2File_i i = mFileMap.find(dict_id);
  if (i == mFileMap.end())
    throw _eh + "dict_id not registered..";
  if (i->second != file)
    throw _eh + "stored file and file passed as argument do not match.";

  // XXXX Who deletes the guy?
  // Does it go to some "Recent" directory? How does this?

  mFileMap.erase(i);
}

XrdFile* XrdServer::FindFile(Int_t dict_id)
{
  mDict2File_i i = mFileMap.find(dict_id);
  return (i != mFileMap.end()) ? i->second : 0;
}
