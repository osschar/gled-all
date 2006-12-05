// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AlContext
//
//

#include "AlContext.h"
#include "AlContext.c7"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

ClassImp(AlContext)

/**************************************************************************/

void AlContext::_init()
{
  // *** Set all links to 0 ***
  mDevice  = 0;
  mContext = 0;
}

/**************************************************************************/

void AlContext::Open()
{
  static const string _eh("AlContext::Open ");
  
  mDevice = alcOpenDevice(0);
  if(mDevice == 0)
    throw(_eh + "can't open device.");

  mContext = alcCreateContext(mDevice, 0);
  if(mContext == 0) {
    alcCloseDevice(mDevice);
    mDevice = 0;
    throw(_eh + "can't create context.");
  }
  alcMakeContextCurrent(mContext);
}