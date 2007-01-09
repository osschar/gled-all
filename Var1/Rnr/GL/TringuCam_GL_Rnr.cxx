// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TringuCam_GL_Rnr.h"
#include <RnrBase/Fl_Event_Enums.h>
#include <Rnr/GL/GLRnrDriver.h>

#include <Opcode/Opcode.h>

#include <GL/gl.h>

/**************************************************************************/

void TringuCam_GL_Rnr::_init()
{}

/**************************************************************************/

void TringuCam_GL_Rnr::PreDraw(RnrDriver* rd)
{}

void TringuCam_GL_Rnr::Draw(RnrDriver* rd)
{}

void TringuCam_GL_Rnr::PostDraw(RnrDriver* rd)
{}

/**************************************************************************/

int TringuCam_GL_Rnr::Handle(RnrDriver* rd, Fl_Event& ev)
{

  int x = ev.fX, y = ev.fY;

  switch (ev.fEvent)
  {

    case FL_PUSH:
    {
      mMouseX = x; mMouseY = y; // reset the drag location

      if (ev.fState & FL_BUTTON1)
      {
        float yext = TMath::Tan(0.5*TMath::DegToRad()*rd->GetZFov())*rd->GetNearClip();
        float xext = yext*rd->GetWidth()/rd->GetHeight();
        float xcam = xext*(2.0f*x/rd->GetWidth()  - 1);
        float ycam = yext*(2.0f*y/rd->GetHeight() - 1);

        mClickPos.SetXYZ(0, 0, 0);
        rd->GetCamFixTrans()->MultiplyIP(mClickPos);
        mTringuCam->RefTrans().MultiplyIP(mClickPos);
        
        mClickDir.SetXYZ(rd->GetNearClip(), -xcam, -ycam);
        mClickDir.SetMag(1);
        rd->GetCamFixTrans()->RotateIP(mClickDir);
        mTringuCam->RefTrans().RotateIP(mClickDir);

        // This should be in TringuCam.
        // Also ... Set-methods for TVector3 sucks! (should take const&)
        // Or use X{r}, or sth. R and r do not work together (can fix p7).
        Tringula& T = * (Tringula*) mTringuCam->GetParent();
        T.SetRayPos(mClickPos);
        T.SetRayDir(mClickDir);
        T.RayCollide();

        // Do some other action?
        // In fact should call TringuCam::UserClickedOrSth

      }

      return 1;
    }

    case FL_DRAG:
    {
      int dx = x - mMouseX, dy = y - mMouseY;

      if (ev.fState & FL_BUTTON2)
      {
        if (dx != 0 || dy != 0) {
          float cfac  = TMath::DegToRad()*rd->GetZFov()/rd->GetHeight();
          float dxang = cfac*dx;
          float dyang = cfac*dy;
          mTringuCam->RotateLF(1, 2, -dxang);
          rd->GetCamFixTrans()->RotateLF(1, 3, -dyang);
          // rd->GetCamFixTrans()->RotateLF(1, 2, -dxang);
          // rd->GetCamFixTrans()->RotateLF(1, 3, -dyang);
          // printf("dx=%d, dy=%d\n", dx, dy);
        }
        
      }
      mMouseX = x; mMouseY = y;
      return 1;
    }

    case FL_KEYDOWN:
    {
      // ev.dump();

      if (mTringuCam->KeyDown(ev.fKey))
        return 1;

      break;
    }

    case FL_KEYUP:
    {
      ev.dump();

      if (mTringuCam->KeyUp(ev.fKey))
        return 1;

      break;
    }


  } // end switch (ev.fEvent)

  // Return 1 to block further processing.
  return 0;
}
