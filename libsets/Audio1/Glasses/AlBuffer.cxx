// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AlBuffer.h"
#include "AlBuffer.c7"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

// AlBuffer

//______________________________________________________________________________
//
//

ClassImp(AlBuffer);

//==============================================================================

void AlBuffer::_init()
{
  mAlBuf = 0;
}

AlBuffer::AlBuffer(const Text_t* n, const Text_t* t) :
  ZGlass(n, t)
{
  _init();
}

AlBuffer::~AlBuffer()
{
  if (mAlBuf) alDeleteBuffers(1, &mAlBuf);
}

//==============================================================================

void AlBuffer::Load()
{
  if (mAlBuf)
  {
    // !!! Should check use count.

    alDeleteBuffers(1, &mAlBuf);
    mAlBuf = 0;
  }

  mAlBuf = alutCreateBufferFromFile(mFile);
  if (mAlBuf == AL_NONE)
    printf("Error in alutCreateBufferFromFile: %s\n", alutGetErrorString(alutGetError()));
}
