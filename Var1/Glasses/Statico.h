// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_Statico_H
#define Var1_Statico_H

#include <Glasses/Extendio.h>

class Statico : public Extendio
{
  friend class Tringula;
  MAC_RNR_FRIENDS(Statico);

private:
  void _init();

protected:

public:
  Statico(const Text_t* n="Statico", const Text_t* t=0) :
    Extendio(n,t) { _init(); }


#include "Statico.h7"
  ClassDef(Statico, 1)
}; // endclass Statico


#endif
