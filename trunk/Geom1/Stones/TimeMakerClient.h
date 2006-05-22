// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_TimeMakerClient_H
#define Geom1_TimeMakerClient_H

#include <Rtypes.h>

class TimeMakerClient
{
public:
  virtual ~TimeMakerClient() {}

  virtual void TimeTick(Double_t time, Double_t delta) = 0;

  ClassDef(TimeMakerClient, 0);
}; // endclass TimeMakerClient

#endif
