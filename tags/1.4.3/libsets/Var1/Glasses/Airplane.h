// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Airplane_H
#define Var1_Airplane_H

#include <Glasses/Flyer.h>

class Airplane : public Flyer
{
  MAC_RNR_FRIENDS(Airplane);

private:
  void _init();

protected:

public:
  Airplane(const Text_t* n="Airplane", const Text_t* t=0);
  virtual ~Airplane();

#include "Airplane.h7"
  ClassDef(Airplane, 1);
}; // endclass Airplane

#endif
