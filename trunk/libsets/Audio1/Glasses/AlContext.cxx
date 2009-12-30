// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AlContext
//
//

#include "AlContext.h"
#include "AlContext.c7"

#include <AL/alut.h>

ClassImp(AlContext);

/**************************************************************************/

void AlContext::_init()
{
  mDevice  = 0;
  mContext = 0;
}

/**************************************************************************/

void AlContext::Open()
{
  static const Exc_t _eh("AlContext::Open ");

  if (mDevice != 0) {
    ISerr(_eh + "device already opened.");
    return;
  }

  mDevice = alcOpenDevice(0);
  if (mDevice == 0)
    throw _eh + "can't open device.";

  mContext = alcCreateContext(mDevice, 0);
  if (mContext == 0) {
    alcCloseDevice(mDevice);
    mDevice = 0;
    throw _eh + "can't create context.";
  }
  alcMakeContextCurrent(mContext);
}

void AlContext::Close()
{
  static const Exc_t _eh("AlContext::Close ");

  if (mContext == 0) return;

  if (alcGetCurrentContext() == mContext)
    alcMakeContextCurrent(0);
  alcDestroyContext(mContext); mContext = 0;
  alcCloseDevice(mDevice);     mDevice  = 0;
}

//==============================================================================
//==============================================================================

// For now, user init is here ... when another piece will need initialization
// it would better be placed somwhere else.

void libAudio1_GLED_user_init()
{
  alutInitWithoutContext(0, 0);
}

void *Audio1_GLED_user_init = (void*)libAudio1_GLED_user_init;

//------------------------------------------------------------------------------

void libAudio1_GLED_user_shutdown()
{
  if ( ! alutExit())
    ISwarn("libAudio1_GLED_user_shutdown alutExit() failed.");
}

void *Audio1_GLED_user_shutdown = (void*)libAudio1_GLED_user_shutdown;
