// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TringuObserverSpiritio_GL_RNR_H
#define Var1_TringuObserverSpiritio_GL_RNR_H

#include <Glasses/TringuObserverSpiritio.h>
#include <Rnr/GL/Spiritio_GL_Rnr.h>

class TringuObserverSpiritio_GL_Rnr : public Spiritio_GL_Rnr
{
private:
  void _init();

protected:
  TringuObserverSpiritio*	mTringuObserverSpiritio;

public:
  TringuObserverSpiritio_GL_Rnr(TringuObserverSpiritio* idol);
  virtual ~TringuObserverSpiritio_GL_Rnr();

  virtual void Draw(RnrDriver* rd);

  virtual int  Handle(RnrDriver* rd, Fl_Event& ev);

}; // endclass TringuObserverSpiritio_GL_Rnr

#endif
