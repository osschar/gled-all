// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AlSource.h"
#include "AlBuffer.h"
#include "AlSource.c7"

#include <Audio1/Audio1.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include <TSystem.h>

//__________________________________________________________________________
// AlSource
//
//

ClassImp(AlSource);

/**************************************************************************/

void AlSource::_init()
{
  mGain    = 1;
  mMinGain = 0;
  mMaxGain = 1;
  mPitch   = 1;

  mConeInnerAngle = 360;
  mConeOuterAngle = 360;
  mConeOuterGain  = 0;

  alGenSources(1, &mAlSrc);
  EmitSourceRay();
  EmitConeRay();
}

AlSource::~AlSource()
{
  if (mAlSrc) alDeleteSources(1, &mAlSrc);
}

//==============================================================================

Bool_t AlSource::IsPlaying()
{
  ALint state = AL_INITIAL;
  alGetSourceiv(mAlSrc, AL_SOURCE_STATE, &state);
  return state == AL_PLAYING || state == AL_PAUSED;
}

//==============================================================================

void AlSource::QueueBuffer(AlBuffer* buf, Int_t count)
{
  static const Exc_t _eh("AlSource::QueueBuffer ");

  if (buf == 0)
    buf = *mDefaultBuffer;

  UInt_t al_buf = 0;
  if (buf != 0)
    al_buf = buf->GetAlBuf();

  while (--count >= 0)
    alSourceQueueBuffers(mAlSrc, 1, &al_buf);

  Audio1::CheckAlError(_eh);
}

void AlSource::Play()
{
  static const Exc_t _eh("AlContext::Play ");

  if (IsPlaying())
      throw _eh + "Already playing.";

  alSourcei(mAlSrc, AL_LOOPING, AL_FALSE);
  alSourcePlay(mAlSrc);
}

void AlSource::Loop()
{
  static const Exc_t _eh("AlSource::Loop ");

  if (IsPlaying())
      throw _eh + "Already playing.";

  alSourcei(mAlSrc, AL_LOOPING, AL_TRUE);
  alSourcePlay(mAlSrc);
}

void AlSource::Stop()
{
  static const Exc_t _eh("AlSource::Stop ");

  if (!IsPlaying())
    throw _eh + "Not playing.";

  alSourceStop(mAlSrc);
}

//==============================================================================

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

//==============================================================================

void AlSource::PrintSourceInfo()
{
  TString out("AlSource::PrintSourceInfo() ");
  out += Identify() + "\n";

  {
    int ii;
    alGetSourcei(mAlSrc, AL_SOURCE_TYPE, &ii);
    TString st;
    if (ii == AL_UNDETERMINED) st = "UNDETERMINED";
    else if (ii == AL_STATIC) st = "STATIC";
    else if (ii == AL_STREAMING) st = "STREAMING";
    else  st = "UNKNOWN";

    TString ss;
    alGetSourcei(mAlSrc, AL_SOURCE_STATE, &ii);
    if      (ii == AL_INITIAL) ss = "INITIAL";
    else if (ii == AL_PLAYING) ss = "PLAYING";
    else if (ii == AL_PAUSED)  ss = "PAUSED";
    else if (ii == AL_STOPPED) ss = "STOPPED";
    else                       ss = "UNKNOWN";

    int lp;
    alGetSourcei(mAlSrc, AL_LOOPING,       &lp);

    out += "  SOURCE_TYPE = " + st + ",  SOURCE_STATE = " + ss + ",  LOOPING = " +
      (lp ? "TRUE" : "FALSE") + "\n";
  }
  {
    int nq, np;
    alGetSourcei(mAlSrc, AL_BUFFERS_QUEUED,    &nq);
    alGetSourcei(mAlSrc, AL_BUFFERS_PROCESSED, &np);
    out += Form("  BUFFERS_QUEUED = %d,  BUFFERS_PROCESSED = %d\n", nq, np);
  }

  cout << out;
}
