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
  mEatNonBoundKeyEvents = true;
}

Spiritio::Spiritio(const Text_t* n, const Text_t* t) :
  ZNode(n, t)
{
  _init();
}

Spiritio::~Spiritio()
{
  for (vKeyInfo_i i = mKeys.begin(); i != mKeys.end(); ++i)
    delete i->fCallback;
}

//==============================================================================

void Spiritio::RegisterKey(KeyHandling::KeyInfo ki)
{
  ki.fIndex = mKeys.size();
  mKeys.push_back(ki);
}

Int_t Spiritio::FindKey(const TString& tag)
{
  for (vKeyInfo_i i = mKeys.begin(); i != mKeys.end(); ++i)
    if (i->fKeyTag == tag)
      return i->fIndex;
  return -1;
}

//==============================================================================

void Spiritio::HandleKey(Int_t key_idx, Bool_t downp, UInt_t time_elapsed)
{
  mKeys[key_idx].fCallback->Invoke(this, key_idx, downp, time_elapsed);
}
