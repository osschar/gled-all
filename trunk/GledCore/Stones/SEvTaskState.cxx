// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// SEvTaskState
//
//

#include "SEvTaskState.h"

ClassImp(SEvTaskState)

/**************************************************************************/

void SEvTaskState::_init()
{
  mState = 'W';
  mNAll = mNOK = mNFail = mNProc = 0;
}

void SEvTaskState::Reinit()
{
  Reinit(mNAll);
}

void SEvTaskState::Reinit(Int_t n)
{
  _init();
  mNAll = n;
}

void SEvTaskState::Finalize()
{
  mNProc = 0;
  mNFail = mNAll - mNOK;
  mState = 'F';
}

/**************************************************************************/

SEvTaskState& SEvTaskState::operator+=(const SEvTaskState& s)
{
  mNAll  += s.mNAll;  mNOK   += s.mNOK;
  mNFail += s.mNFail; mNProc += s.mNProc;
  return *this;
}


/**************************************************************************/
