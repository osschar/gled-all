// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuObserverSpiritio_GL_Rnr.h"
#include <Glasses/TringuCam.h>
#include <Rnr/GL/GLRnrDriver.h>
#include <RnrBase/Fl_Event_Enums.h>

#include <Glasses/Camera.h>

#include <TMath.h>

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

// void TringuObserverSpiritio_GL_Rnr::Draw(RnrDriver* rd) {}

//==============================================================================

int TringuObserverSpiritio_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  // For now just forward to TringuCam_GL_Rnr ...

  TringuCam &TC = * mTringuObserverSpiritio->GetTringuCam();

  switch (ev.fEvent)
  {

    case FL_PUSH:
    {
      if (ev.fButton == 1)
      {
        TC.MouseDown(ev);
      }

      return 1;
    }

    case FL_DRAG:
    {
      if (ev.fState & FL_BUTTON2)
      {
	int dx = rd->GL()->GetMDragDX();
	int dy = rd->GL()->GetMDragDY();
        float cfac = TMath::DegToRad()*rd->GetZFov()/rd->GetHeight();
        if (dx != 0)
        {
          float dxang =  cfac * dx;
          TC.RotateLF(1, 2, -dxang);
        }
        if (dy != 0)
        {
          float dyang = -cfac * dy;
          ZTrans& t = mTringuObserverSpiritio->GetCamera()->ref_trans();
          // Do not rotate beyond vertical.
          Float_t d = t.ArrX()[2];
          if (d > 1) d = 1; else if(d < -1) d = -1; // Fix numerical errors
          Float_t a = TMath::ASin(d) + dyang;
          if (a >= TMath::PiOver2() && dyang > 0)
          {
            t.SetBaseVec(1, 0, 0, 1);
            t.OrtoNorm3Column(3, 1);
            t.SetBaseVecViaCross(2);
          }
          else if (a <= -TMath::PiOver2() && dyang < 0)
          {
            t.SetBaseVec(1, 0, 0, -1);
            t.OrtoNorm3Column(3, 1);
            t.SetBaseVecViaCross(2);
          }
          else
          {
            t.RotateLF(1, 3, dyang);
          }
        }
      }
      return 1;
    }

    case FL_RELEASE:
    {
      if (ev.fButton == 1)
      {
        TC.MouseUp();
      }

      return 1;
    }

    case FL_KEYDOWN:
    {
      if (TC.GetKeysVerbose()) ev.dump();

      if (TC.KeyDown(ev.fKey))
        return 1;

      break;
    }

    case FL_KEYUP:
    {
      if (TC.GetKeysVerbose()) ev.dump();

      if (TC.KeyUp(ev.fKey))
        return 1;

      break;
    }

  } // end switch (ev.fEvent)

  // Return 1 to block further processing.
  return 0;
}
