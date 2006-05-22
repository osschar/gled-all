// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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

ClassImp(AlSource)

/**************************************************************************/

void AlSource::_init()
{
  // *** Set all links to 0 ***
  mFile = "boom.wav";

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

  if(alIsSource(sid) == AL_FALSE) {
    return AL_FALSE;
  }

  state = AL_INITIAL;
  alGetSourceiv(sid, AL_SOURCE_STATE, &state);
  switch(state) {
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

    if(mAlBuf == 0) alGenBuffers(1, &mAlBuf);
    if(mAlSrc == 0) {
      alGenSources(1, &mAlSrc);
      EmitSourceRay();
      EmitConeRay();
    }

    // --------------------------------------------------------------
    // ogg loader
    /*
      {
      struct stat sbuf;

      if(stat(mFile.Data(), &sbuf) == -1) {
      perror(mFile.Data());
      throw(_eh + "stat failed.");
      }
  
      int   size = sbuf.st_size;
      char* data = new char[size];

      FILE* fh = fopen(mFile.Data(), "r");
      if(fh == NULL) {
      delete [] data;
      throw(_eh + "could not open file.");
      }
      fread(data, size, 1, fh);

      vorbisLoader *alutLoadVorbisp =
      (vorbisLoader *) alGetProcAddress((ALubyte *) "alutLoadVorbis_LOKI");
      if(alutLoadVorbisp == 0) {
      delete [] data;
      throw(_eh + "could not GetProc.");
      }

      if(alutLoadVorbisp(mAlBuf, data, size) != AL_TRUE) {
      delete [] data;
      throw(_eh + "alutLoadVorbis failed.");
      }

      delete [] data;
      }
    */

    // --------------------------------------------------------------
    // wav loader
    {
      void*   wave = NULL;
      ALsizei size;
      ALsizei freq;
      ALenum  format;
      ALboolean loop;

      alutLoadWAVFile((ALbyte*) mFile.Data(),
			    &format, &wave, &size, &freq, &loop);
      if(alutGetError())
	throw(_eh + "could not open file.");

      alBufferData(mAlBuf, format, wave, size, freq );
      free(wave); /* openal makes a local copy of wave data */
    }

    // --------------------------------------------------------------

    if(count > 0) {
      alSourcei(mAlSrc, AL_LOOPING, AL_FALSE);
      while(count--) {
	alSourceQueueBuffers(mAlSrc, 1, &mAlBuf);
	// alQueuei(mAlSrc, 1, mAlBuf);
      }
    } else {
      alSourcei(mAlSrc, AL_LOOPING, AL_TRUE);
      alSourcei(mAlSrc, AL_BUFFER, mAlBuf);
    }
  }

  // GTime time(GTime::I_Now);
  alSourcePlay(mAlSrc);
  // source_info(mAlSrc, "play.");
  while(SourceIsPlaying(mAlSrc) == AL_TRUE) {
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
  if(mAlSrc) {
    alSourcef(mAlSrc, AL_GAIN, mGain);
    alSourcef(mAlSrc, AL_MIN_GAIN, mMinGain);
    alSourcef(mAlSrc, AL_MAX_GAIN, mMaxGain);
    alSourcef(mAlSrc, AL_PITCH, mPitch);
  }
}

void AlSource::EmitConeRay()
{
  if(mAlSrc) {
    alSourcef(mAlSrc, AL_CONE_INNER_ANGLE, mConeInnerAngle);
    alSourcef(mAlSrc, AL_CONE_OUTER_ANGLE, mConeOuterAngle);
    alSourcef(mAlSrc, AL_CONE_OUTER_GAIN,  mConeOuterGain);
  }
}
