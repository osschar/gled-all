// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// MultiSpheror
//
//

#include "MultiSpheror.h"
#include "Spheror.h"
#include <Glasses/ZQueen.h>
#include <Gled/GledMirDefs.h>

#include <TRandom.h>



ClassImp(MultiSpheror)

/**************************************************************************/

void MultiSpheror::_init()
{
  mNVert = 20;
  mDx = 3;
  mNGrid = 4;
  mNG = mNtoDo = mNDone = 0;
}

/**************************************************************************/

namespace {
  template <class T>
  T NextOf(lpZGlass_i& i, lpZGlass_i& end)
  {
    T ret = 0;
    do {
      if(++i == end) break;
      ret = dynamic_cast<T>(*i);
    } while(ret == 0);
    return ret;
  }
};

/**************************************************************************/

void MultiSpheror::Init()
{
  if(mNG != 0) {
    mListMutex.Lock();
    lpZGlass_i spheri = mGlasses.begin();
    Spheror* spheror = 0;
    while(spheri != mGlasses.end()) {
      spheror = dynamic_cast<Spheror*>(*spheri);
      if(spheror) {
	spheror->GetAmoeba()->SetHost(0);
	spheror->GetAmoeba()->SetWA_Master(0);
	spheror->SetAmoeba(0);
      }
      ++spheri;
    }
    mListMutex.Unlock();
    ClearList();
    mNG = 0;
  }

  mNG = mNGrid; mNtoDo = mNG*mNG; mNDone = 0;

  // This will give different state on different nodes. But it is just
  // to have sth to draw. Spherors will be reinited upon thread start-up.
  TRandom rnd(0);

  for(UShort_t i=0; i<mNtoDo; ++i) {
    UShort_t ki = i % mNG;
    UShort_t kj = i / mNG;

    Spheror* s = new Spheror(GForm("S(%hu,%hu)", ki, kj));
    s->MoveLF(1, mDx*(ki - 0.5*mNG));
    s->MoveLF(2, mDx*(kj - 0.5*mNG));
    s->SetNVert(mNVert);
    s->SetBeautyP(true);
    s->SetColor(1, 1, 0.5);
    auto_ptr<TVectorF> v(s->InitialState(rnd));
    s->SetState(*v);

    mQueen->CheckIn(s); s->Install(); Add(s);
    // Could get first ... and torture it further ...
  }
  Stamp(FID());
}

void MultiSpheror::ClaimCPUs()
{
  if(IsSunSpace()) {
    if(mNDone >= mNtoDo) {
      throw(string(GForm("MultiSpheror::ClaimCPUs [%s] finished.", GetName())));
    }

    mListMutex.Lock();
    lpZGlass_i spheri = mGlasses.begin();
    Spheror* spheror = 0;
    while(spheri != mGlasses.end()) {
      spheror = dynamic_cast<Spheror*>(*spheri);
      if(spheror && spheror->GetAmoeba()->GetHost() == 0) break;
      ++spheri;
    }
    if(spheror == 0) {
      throw(string("MultiSpheror::ClaimCPUs no spherors found."));
    }

    lpZGlass_t sats;
    mSaturn->GetSaturnInfo()->GetMoons()->Copy(sats);
    sats.push_back(mSaturn->GetSaturnInfo());
    for(lpZGlass_i s=sats.begin(); s!=sats.end(); ++s) {
      if(mNDone >= mNtoDo) break;
      SaturnInfo* si = dynamic_cast<SaturnInfo*>(*s);
      if(si && si->GetLAvg5() < si->GetCPU_Num()) {
	if(spheror == 0 || spheror->GetAmoeba() == 0) {
	  mListMutex.Unlock();
	  throw(string("MultiSpheror::ClaimCPUs Improper spheror structure. Abandoning."));
	}
	SP_MIR(spheror->GetAmoeba(), SetHost, si);
	SP_MIR(spheror->GetAmoeba(), Start);
	++mNDone; Stamp(FID());
	lpZGlass_i xx = mGlasses.end();
	spheror = NextOf<Spheror*>(spheri, xx);
      }
    }

    mListMutex.Unlock();
  }
}

/**************************************************************************/

#include "MultiSpheror.c7"
