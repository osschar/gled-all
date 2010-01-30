// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuObserverSpiritio.h"
#include <Glasses/ZNode.h>
#include "TringuCam.h"
#include "TringuObserverSpiritio.c7"

#include <Glasses/Camera.h>
#include <Glasses/ZQueen.h>

// TringuObserverSpiritio

//______________________________________________________________________________
//
// Right now ... just forward all to TringuCam, also in GL-Rnr.
// Some functionality of TringuCam will come here.

ClassImp(TringuObserverSpiritio);

//==============================================================================

void TringuObserverSpiritio::_init()
{}

TringuObserverSpiritio::TringuObserverSpiritio(const Text_t* n, const Text_t* t) :
  Spiritio(n, t)
{
  _init();
}

TringuObserverSpiritio::~TringuObserverSpiritio()
{}

//==============================================================================

void TringuObserverSpiritio::AdEnlightenment()
{
  // Create the camera.

  PARENT_GLASS::AdEnlightenment();

  if (mCamera == 0)
  {
    Camera* c = new Camera("TringuObserverCamera", "Camera of TringuObserverSpiritio");
    c->SetMIRActive(false);
    mQueen->CheckIn(c);
    SetCamera(c);
  }
}

//==============================================================================

void TringuObserverSpiritio::Activate()
{
  mCamera->SetParent(*mTringuCam);
  // mCamera->Home();

  PARENT_GLASS::Activate();
}

void TringuObserverSpiritio::Deactivate()
{
  PARENT_GLASS::Deactivate();

  mCamera->SetParent(0);
}

//==============================================================================

void TringuObserverSpiritio::TimeTick(Double_t t, Double_t dt)
{
  mTringuCam->TimeTick(t, dt);
}
