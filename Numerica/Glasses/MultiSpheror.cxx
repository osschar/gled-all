// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// MultiSpheror
//
//

#include "MultiSpheror.h"
#include "MultiSpheror.c7"

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

void MultiSpheror::Init()
{
  if(mNG != 0) {
    mListMutex.Lock();
    Stepper<Spheror> s(this);
    while(s.step()) {
      s->GetAmoeba()->SetHost(0);
      s->GetAmoeba()->SetWA_Master(0);
      s->SetAmoeba(0);
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
  static const Exc_t _eh("MultiSpheror::ClaimCPUs ");

  if(IsSunSpace() == false)
    return;

  if(mNDone >= mNtoDo) {
    throw(_eh + Identify() + "finished.");
  }

  list<Spheror*> todo;
  {
    GMutexHolder llck(mListMutex);
    Stepper<Spheror> s(this);
    while(s.step()) {
      if(s->GetAmoeba()->GetHost() == 0)
	todo.push_back(*s);
    }
  }
  if(todo.empty())
    throw(_eh + "nothing to do.");

  lpZGlass_t sats;
  mSaturn->GetSaturnInfo()->GetMoons()->CopyList(sats);
  sats.push_back(mSaturn->GetSaturnInfo());
  for(lpZGlass_i s=sats.begin(); s!=sats.end(); ++s) {
    if(mNDone >= mNtoDo) break;
    SaturnInfo* si = dynamic_cast<SaturnInfo*>(*s);
    if(si && si->GetLAvg5() < si->GetCPU_Num()) {
      Spheror* spheror = todo.front();
      SP_MIR(spheror->GetAmoeba(), SetHost, si);
      SP_MIR(spheror->GetAmoeba(), Start);
      ++mNDone; Stamp(FID());
      todo.pop_front();
    }
  }

}
