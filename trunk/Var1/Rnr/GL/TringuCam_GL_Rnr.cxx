// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuCam_GL_Rnr.h"
#include <RnrBase/Fl_Event_Enums.h>
#include <Rnr/GL/GLRnrDriver.h>

#include <Opcode/Opcode.h>

#include <TMath.h>

#include <GL/gl.h>

/**************************************************************************/

void TringuCam_GL_Rnr::_init()
{}

/**************************************************************************/

void TringuCam_GL_Rnr::PreDraw(RnrDriver* rd)
{}

void TringuCam_GL_Rnr::Draw(RnrDriver* rd)
{
  TringuCam& TC = * mTringuCam;
  TC.mCamFix  = rd->GetCamFixTrans();
  TC.mScreenW = rd->GetWidth();
  TC.mScreenH = rd->GetHeight();
  TC.mNearClp = rd->GetNearClip();
  TC.mFarClp  = rd->GetFarClip();
  TC.mZFov    = rd->GetZFov();
}

void TringuCam_GL_Rnr::PostDraw(RnrDriver* rd)
{}

/**************************************************************************/

int TringuCam_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{
  int x = ev.fX, y = ev.fY;

  TringuCam& TC = *mTringuCam;
  TC.mMouseX = x;
  TC.mMouseY = y;

  switch (ev.fEvent)
  {

    case FL_PUSH:
    {
      TC.mMPushX = mMouseX = x;
      TC.mMPushY = mMouseY = y; // reset the drag location

      if (ev.fButton == 1)
      {
        TC.MouseDown();
      }

      return 1;
    }

    case FL_DRAG:
    {
      int dx = x - mMouseX, dy = y - mMouseY;
      TC.mMDrgDX = dx;
      TC.mMDrgDY = dy;

      if (ev.fState & FL_BUTTON2)
      {
        if (dx != 0 || dy != 0) {
          float cfac  = TMath::DegToRad()*rd->GetZFov()/rd->GetHeight();
          float dxang = cfac*dx;
          float dyang = cfac*dy;
          mTringuCam->RotateLF(1, 2, -dxang);
          rd->GetCamFixTrans()->RotateLF(1, 3, -dyang);
        }
        
      }

      mMouseX = x; mMouseY = y;
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
      if (mTringuCam->GetKeysVerbose()) ev.dump();

      if (mTringuCam->KeyDown(ev.fKey))
        return 1;

      break;
    }

    case FL_KEYUP:
    {
      if (mTringuCam->GetKeysVerbose()) ev.dump();

      if (mTringuCam->KeyUp(ev.fKey))
        return 1;

      break;
    }

  } // end switch (ev.fEvent)

  // Return 1 to block further processing.
  return 0;
}
