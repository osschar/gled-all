// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "CosmicBall.h"
#include "CosmicBall.c7"

// CosmicBall

//__________________________________________________________________________
//
//

ClassImp(CosmicBall);

/**************************************************************************/

void CosmicBall::_init()
{
  mHistorySize = mHistoryFirst = mHistoryStored = 0;
}

/**************************************************************************/

void CosmicBall::StorePos()
{
  GMutexHolder histo_lock(mHistoryMoo);

  if (mHistoryFirst == 0)
  {
    mHistoryFirst = mHistorySize;
  }
  --mHistoryFirst;

  mHistory[mHistoryFirst].Set(mTrans.ArrT());

  if (mHistoryStored < mHistorySize)
    ++mHistoryStored;
}

void CosmicBall::ResizeHistory(Int_t size)
{
  GMutexHolder histo_lock(mHistoryMoo);

  if (size == mHistorySize)
    return;

  vector<Point> hist(size);

  Int_t n_to_copy = TMath::Min(mHistoryStored, size);

  if (n_to_copy > 0)
  {
    Int_t lim_up = TMath::Min(mHistoryFirst + n_to_copy, mHistorySize);
    Int_t n_copy = lim_up - mHistoryFirst;
    memcpy(&hist[0], &mHistory[mHistoryFirst], n_copy * sizeof(Point));

    n_to_copy -= n_copy;

    if (n_to_copy > 0)
    {
      memcpy(&hist[n_copy], &mHistory[0], n_to_copy * sizeof(Point));
    }
  }

  mHistory.swap(hist);

  mHistorySize   = size;
  mHistoryFirst  = 0;
  mHistoryStored = TMath::Min(mHistoryStored, size);
}

void CosmicBall::ClearHistory()
{
  GMutexHolder histo_lock(mHistoryMoo);

  mHistoryStored = 0;
}

/**************************************************************************/
