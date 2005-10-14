// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_TimeScreenText_H
#define Geom1_TimeScreenText_H

#include <Glasses/ScreenText.h>

#include <Stones/TimeMakerClient.h>

class TimeScreenText : public ScreenText, public TimeMakerClient
{
  MAC_RNR_FRIENDS(TimeScreenText);

private:
  void _init();

protected:
  TString       mFormat; // X{GS} 7 Textor()

public:
  TimeScreenText(const Text_t* n="TimeScreenText", const Text_t* t=0) :
    ScreenText(n,t) { _init(); }

  // TimeMakerClient
  virtual void TimeTick(Double_t t, Double_t dt);

#include "TimeScreenText.h7"
  ClassDef(TimeScreenText, 1);
}; // endclass TimeScreenText


#endif
