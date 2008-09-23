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
{}

/**************************************************************************/

void CosmicBall::ResizeHistory(Int_t max_size)
{
  if (max_size < 1)
  {
    if ( ! mHistory.empty())
      mHistory.clear();
  }
  else
  {
    if (mHistory.size() > max_size)
      mHistory.resize(max_size);
  }
}

void CosmicBall::StorePos(Int_t max_size)
{
  mHistory.push_front(Point(mTrans.ArrT()));

  if (mHistory.size() > max_size)
    mHistory.resize(max_size);
}

/**************************************************************************/
