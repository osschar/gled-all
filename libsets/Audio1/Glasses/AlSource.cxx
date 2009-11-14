// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AlSource
//
//

#include "AlSource.h"
#include "AlSource.c7"
#include <RnrBase/RnrDriver.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

ClassImp(AlSource);

/**************************************************************************/

void AlSource::_init()
{
  mFile = "";

  mGain    = 1;
  mMinGain = 0;
  mMaxGain = 1;
  mPitch   = 1;

  mConeInnerAngle = 360;
  mConeOuterAngle = 360;
  mConeOuterGain  = 0;

  mAlBuf = 0;
  mAlSrc = 0;
}

AlSource::~AlSource()
{
  if(mAlSrc) alDeleteSources(1, &mAlSrc);
  if(mAlBuf) alDeleteBuffers(1, &mAlBuf);
}

/**************************************************************************/

typedef ALboolean (vorbisLoader)(ALuint, ALvoid *, ALint);

#include <TSystem.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

ALboolean SourceIsPlaying(ALuint sid)
{
  ALint state;

  if (alIsSource(sid) == AL_FALSE)
    return AL_FALSE;

  state = AL_INITIAL;
  alGetSourceiv(sid, AL_SOURCE_STATE, &state);
  switch (state)
  {
    case AL_PLAYING:
    case AL_PAUSED:
      return AL_TRUE;
    default:
      break;
  }

  return AL_FALSE;
}

void source_info(ALuint vorbsource, const Text_t* foo)
{
  int nq, np;
  alGetSourcei(vorbsource, AL_BUFFERS_QUEUED, &nq);
  alGetSourcei(vorbsource, AL_BUFFERS_PROCESSED, &np);
  printf("%d %s (%d,%d)\n", vorbsource, foo, nq, np);
}

void AlSource::Play(Int_t count)
{
  static const Exc_t _eh("AlContext::Play ");

  {
    GLensReadHolder _rdlck(this);

    // if(mAlBuf == 0) alGenBuffers(1, &mAlBuf);
    if (mAlSrc == 0)
    {
      alGenSources(1, &mAlSrc);
      EmitSourceRay();
      EmitConeRay();
    }

    // --------------------------------------------------------------
    // wav loader
    {
      mAlBuf = alutCreateBufferFromFile(mFile);

      if (mAlBuf == AL_NONE)
	printf("Error in alutCreateBufferFromFile: %s\n", alutGetErrorString(alutGetError()));
    }

    // --------------------------------------------------------------

    if (count > 0)
    {
      alSourcei(mAlSrc, AL_LOOPING, AL_FALSE);
      while (count--)
      {
	alSourceQueueBuffers(mAlSrc, 1, &mAlBuf);
	// alQueuei(mAlSrc, 1, mAlBuf);
      }
    }
    else
    {
      alSourcei(mAlSrc, AL_LOOPING, AL_TRUE);
      alSourcei(mAlSrc, AL_BUFFER, mAlBuf);
    }
  }

  // GTime time(GTime::I_Now);
  alSourcePlay(mAlSrc);
  // source_info(mAlSrc, "play.");
  while (SourceIsPlaying(mAlSrc) == AL_TRUE)
  {
    // source_info(mAlSrc, "sleep.");
    gSystem->Sleep(100);
  }
  // time = time.TimeUntilNow();
  // printf("Time used %lfs.\n", time.ToDouble());
  // source_info(mAlSrc, "done.");

  GLensReadHolder _rdlck(this);

  alDeleteBuffers(1, &mAlBuf); mAlBuf = 0;
  alDeleteSources(1, &mAlSrc); mAlSrc = 0;
}

/**************************************************************************/

void AlSource::EmitSourceRay()
{
  if (mAlSrc)
  {
    alSourcef(mAlSrc, AL_GAIN, mGain);
    alSourcef(mAlSrc, AL_MIN_GAIN, mMinGain);
    alSourcef(mAlSrc, AL_MAX_GAIN, mMaxGain);
    alSourcef(mAlSrc, AL_PITCH, mPitch);
  }
}

void AlSource::EmitConeRay()
{
  if (mAlSrc)
  {
    alSourcef(mAlSrc, AL_CONE_INNER_ANGLE, mConeInnerAngle);
    alSourcef(mAlSrc, AL_CONE_OUTER_ANGLE, mConeOuterAngle);
    alSourcef(mAlSrc, AL_CONE_OUTER_GAIN,  mConeOuterGain);
  }
}
