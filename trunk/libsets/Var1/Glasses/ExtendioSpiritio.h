// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_ExtendioSpiritio_H
#define Var1_ExtendioSpiritio_H

#include <Glasses/Spiritio.h>

class Extendio;

class ExtendioSpiritio : public Spiritio
{
  MAC_RNR_FRIENDS(ExtendioSpiritio);

private:
  void _init();

protected:
  ZLink<Extendio> mExtendio;     // X{GS} L{f}
  FID_t           mExtendio_fid; //!

public:
  ExtendioSpiritio(const Text_t* n="ExtendioSpiritio", const Text_t* t=0);
  virtual ~ExtendioSpiritio();

  // Should keep abstract?
  virtual void TimeTick(Double_t t, Double_t dt) {}

#include "ExtendioSpiritio.h7"
  ClassDef(ExtendioSpiritio, 1);
}; // endclass ExtendioSpiritio

#endif
