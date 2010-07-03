// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "XTReqCanvas.h"

#include "TCanvas.h"
#include "TSystem.h"
#include "TClass.h"

// XTReqCanvas

//______________________________________________________________________________
//
// Create canvas in cross-thread request to ROOT thread.

ClassImp(XTReqCanvas);

//==============================================================================

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

  // This bitch was often crashing at random sub-places down
  // from TQObject::CollectClassSignalLists(), when
  // emitting TGFrame::ProcessedEvent() for the first time.
  // Go figure ...
  TClass::GetClass("TGHorizontalFrame", true, false);
}
