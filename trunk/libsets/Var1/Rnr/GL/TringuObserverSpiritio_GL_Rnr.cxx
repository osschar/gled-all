// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuObserverSpiritio_GL_Rnr.h"
#include <GL/glew.h>

//==============================================================================

void TringuObserverSpiritio_GL_Rnr::_init()
{}

TringuObserverSpiritio_GL_Rnr::TringuObserverSpiritio_GL_Rnr(TringuObserverSpiritio* idol) :
  Spiritio_GL_Rnr(idol),
  mTringuObserverSpiritio(idol)
{
  _init();
}

TringuObserverSpiritio_GL_Rnr::~TringuObserverSpiritio_GL_Rnr()
{}

//==============================================================================

void TringuObserverSpiritio_GL_Rnr::Draw(RnrDriver* rd) {}

//==============================================================================

int TringuObserverSpiritio_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  // A_Rnr* tcrnr = rd->GetLensRnr(mTringuObserverSpiritio->???);
  return 0;
}
