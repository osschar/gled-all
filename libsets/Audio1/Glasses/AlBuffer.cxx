// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AlBuffer.h"
#include "AlBuffer.c7"

#include <AL/alut.h>

// AlBuffer

//______________________________________________________________________________
//
//

ClassImp(AlBuffer);

//==============================================================================

void AlBuffer::clear_buffer_props()
{
  mFrequency = mSize = mBits = mChannels = 0;
  mDuration = 0;
}

void AlBuffer::_init()
{
  mAlBuf = 0;
  clear_buffer_props();
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
    // !!! Should check use count - the call fails if buffer is still used.
    // !!! But it is not set from AlSource, so think a bit.

    alDeleteBuffers(1, &mAlBuf);
    mAlBuf = 0;
  }

  mAlBuf = alutCreateBufferFromFile(mFile);

  if (mAlBuf != AL_NONE)
  {
    alGetBufferi(mAlBuf, AL_FREQUENCY, &mFrequency);
    alGetBufferi(mAlBuf, AL_SIZE,      &mSize);
    alGetBufferi(mAlBuf, AL_BITS,      &mBits);
    alGetBufferi(mAlBuf, AL_CHANNELS,  &mChannels);
    mDuration = (8.0f * mSize) / (mFrequency * mBits * mChannels);
  }
  else
  {
    clear_buffer_props();
    printf("Error in alutCreateBufferFromFile: %s\n", alutGetErrorString(alutGetError()));
  }

  Stamp(FID());
}
