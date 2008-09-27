// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// LocalMover
//
//

#include "LocalMover.h"
#include "LocalMover.c7"

#include <Stones/ZTrans.h>

ClassImp(LocalMover);

/**************************************************************************/

void LocalMover::_init()
{
  bMoveOn   = true;
  bMoveInPF = false;
  mDx = mDy = mDz = 0;

  bRotOn   = true;
  bRotInPF = false;
  mPhi = mTheta = mEta = 0;

  mRotMatrix = 0;
}

/**************************************************************************/


/**************************************************************************/
