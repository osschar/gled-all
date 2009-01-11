// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Chopper_H
#define Var1_Chopper_H

#include <Glasses/Flyer.h>

class Chopper : public Flyer
{
  MAC_RNR_FRIENDS(Chopper);

private:
  void _init();

protected:

public:
  Chopper(const Text_t* n="Chopper", const Text_t* t=0);
  virtual ~Chopper();

#include "Chopper.h7"
  ClassDef(Chopper, 1);
}; // endclass Chopper

#endif
