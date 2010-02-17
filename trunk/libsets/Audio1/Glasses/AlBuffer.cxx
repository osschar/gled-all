// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AlBuffer.h"
#include "AlBuffer.c7"

#include <AL/alut.h>
#include <vorbis/vorbisfile.h>

namespace
{
#ifdef R__BYTESWAP
  static const int slBigEndian = 0;
#else
  static const int slBigEndian = 1;
#endif
}

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
  static const Exc_t _eh("AlBuffer::Load ");

  if (mAlBuf)
  {
    // !!! Should check use count - the call fails if buffer is still used.
    // !!! But it is not set from AlSource, so think a bit.

    alDeleteBuffers(1, &mAlBuf);
    mAlBuf = 0;
  }

  if (mFile.EndsWith(".wav"))
  {
    mAlBuf = alutCreateBufferFromFile(mFile);
    if (mAlBuf == AL_NONE)
    {
      ISerr(_eh + GForm("Error in alutCreateBufferFromFile: %s\n", alutGetErrorString(alutGetError())));
    }
  }
  else if (mFile.EndsWith(".ogg"))
  {
    FILE *fp = fopen(mFile, "r");
    if (fp)
    {
      OggVorbis_File oggFile;
      int status = ov_open(fp, &oggFile, 0, 0);
      if (status == 0)
      {
	vorbis_info *info = ov_info(&oggFile, -1);
	ogg_int64_t  len = ov_pcm_total(&oggFile, -1) * info->channels * 2;
	UChar_t     *data = new UChar_t[len];

	int          bs    = -1;
	ogg_int64_t  todo  = len;
	UChar_t     *bufpt = data;

	while (todo)
	{
	  long read = ov_read(&oggFile, (char*)bufpt, todo, slBigEndian, 2, 1, &bs);
	  todo  -= read;
	  bufpt += read;
	}

	alGenBuffers(1, &mAlBuf);
	alBufferData(mAlBuf, (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data, len, info->rate);

	delete [] data;
	ov_clear(&oggFile);
      }
      else
      {
	const char *err;
	switch (status)
	{
	case OV_EREAD:      err = "A read from media returned an error."; break;
	case OV_ENOTVORBIS: err = "Bitstream does not contain any Vorbis data."; break;
	case OV_EVERSION:   err = "Vorbis version mismatch."; break;
	case OV_EBADHEADER: err = "Invalid Vorbis bitstream header."; break;
	case OV_EFAULT:     err = "Internal logic fault; indicates a bug or heap/stack corruption."; break;
	default:            err = "unknown."; break;
	}
	ISerr(_eh + "Error in OGG ov_open: " + err);
	fclose(fp);
      }
    }
    else
    {
      ISerr(_eh + "Can not open file.");
    }
  }
  else
  {
    ISerr(_eh + "Unknown file-type.");
  }

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
  }

  Stamp(FID());
}
