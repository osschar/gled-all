// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SSocket.h"

// SSocket

//______________________________________________________________________________
//
// Wrapper over TSocket avoiding double invocation of Close() from reader and
// writer threads.

ClassImp(SSocket);

//==============================================================================

void SSocket::Close(Option_t *opt)
{
  GMutexHolder _lck(mMutex);
  if (!mClosedDown)
  {
    TSocket::Close(opt);
    mClosedDown = true;
  }
}
