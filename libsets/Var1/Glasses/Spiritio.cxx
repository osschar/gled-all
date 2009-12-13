// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Spiritio.h"
#include <Glasses/Camera.h>
#include <Glasses/TSPupilInfo.h>
#include "Spiritio.c7"

// Spiritio

//______________________________________________________________________________
//
//

ClassImp(Spiritio);

//==============================================================================

UInt_t Spiritio::sDoubleClickTime = 300;

void Spiritio::_init()
{
  mEatNonBoundKeyEvents = false;
}

Spiritio::Spiritio(const Text_t* n, const Text_t* t) :
  ZNode(n, t)
{
  _init();
}

Spiritio::~Spiritio()
{
  for (vpKeyInfo_i i = mKeys.begin(); i != mKeys.end(); ++i)
    delete *i;
}

//==============================================================================

KeyHandling::KeyInfo&
Spiritio::RegisterKey(const TString& tag, const TString& desc,
		      KeyHandling::AKeyCallback* foo)
{
  using namespace KeyHandling;

  KeyInfo* ki = new KeyInfo(tag, desc, foo);
  ki->fIndex = mKeys.size();
  mKeys.push_back(ki);
  return *ki;
}

Int_t Spiritio::FindKey(const TString& tag)
{
  for (vpKeyInfo_i i = mKeys.begin(); i != mKeys.end(); ++i)
  {
    if ((*i)->fKeyTag == tag)
      return (*i)->fIndex;
  }
  return -1;
}

//==============================================================================

void Spiritio::Activate()
{
  bActive = true;
  Stamp(FID());
}

void Spiritio::Deactivate()
{
  // Releases all keys by looping over key-infos and calling the key-up
  // callback the necessary number of times.
  // PRQN_release_keys ray is emitted to also notify the GL renderers.

  if (!bActive)
    return;

  bActive = false;
  Stamp(FID());

  ReleaseAllKeys();
}

//==============================================================================

void Spiritio::HandleKey(Int_t key_idx, Bool_t downp, UInt_t time_elapsed)
{
  if (bActive)
  {
    mKeys[key_idx]->fCallback->Invoke(this, key_idx, downp, time_elapsed);
  }
}

//==============================================================================

void Spiritio::ReleaseAllKeys()
{
  for (vpKeyInfo_i i = mKeys.begin(); i != mKeys.end(); ++i)
  {
    while ((*i)->fDownCount)
    {
      (*i)->fCallback->Invoke(this, (*i)->fIndex, false, 0);
    }
  }

  if (mQueen && mSaturn->AcceptsRays())
  {
    auto_ptr<Ray> ray
      (Ray::PtrCtor(this, PRQN_release_keys, mTimeStamp, FID()));
    mQueen->EmitRay(ray);
  }
}
