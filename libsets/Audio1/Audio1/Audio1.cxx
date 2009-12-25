// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Audio1.h"

#include <AL/alut.h>

using namespace Audio1;

void Audio1::CheckAlError(const Exc_t& eh)
{
  ALenum err = alGetError();

  if (err != AL_NO_ERROR)
  {
    switch (err)
    {
      case AL_INVALID_NAME:
	ISerr(eh + "Invalid name parameter.");
	break;
      case AL_INVALID_ENUM:
	ISerr(eh + "Invalid parameter.");
	break;
      case AL_INVALID_VALUE:
	ISerr(eh + "Invalid enum parameter value.");
	break;
      case AL_INVALID_OPERATION:
	ISerr(eh + "Illegal call.");
	break;
      case AL_OUT_OF_MEMORY:
	ISerr(eh + "Unable to allocate memory.");
	break;
      default:
	ISerr(eh + "Unknown error.");
	break;
    }
  }
}
