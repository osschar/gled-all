// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XTReqCanvas.h"

#include "TCanvas.h"
#include "TSystem.h"
#include "TClass.h"

//==============================================================================
// XTReqCanvas
//------------------------------------------------------------------------------
// Create canvas in cross-thread request to ROOT thread.
//------------------------------------------------------------------------------

void XTReqCanvas::Act()
{
  fCanvas = new TCanvas(fName, fTitle, fW, fH);
  if (fCanvas && (fNPx > 1 || fNPy > 1))
  {
    fCanvas->Divide(fNPx, fNPy);
    fCanvas->Update();
  }

  // Make sure window is on screen before we pass it back ...
  gSystem->ProcessEvents();
}

TCanvas* XTReqCanvas::Request(const char* name, const char* title,
			      int w, int h, int npx, int npy)
{
  auto_ptr<XTReqCanvas> creq(new XTReqCanvas(name, title, w, h, npx, npy));
  creq->ShootRequestAndWait();
  return creq->fCanvas;
}


//==============================================================================
// XTReqPadUpdate
//------------------------------------------------------------------------------
// Create canvas in cross-thread request to ROOT thread.
//------------------------------------------------------------------------------

void XTReqPadUpdate::Act()
{
  fPad->Modified();
  fPad->Update();
}

void XTReqPadUpdate::Update(TVirtualPad* p)
{
  (new XTReqPadUpdate(p))->ShootRequest();
}
