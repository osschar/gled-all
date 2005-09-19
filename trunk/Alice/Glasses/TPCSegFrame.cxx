// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// TPCSegFrame
//
//

#include "TPCSegFrame.h"
#include "TPCSegFrame.c7"

ClassImp(TPCSegFrame)

/**************************************************************************/

  void TPCSegFrame::_init()
{
  // *** Set all links to 0 ***
  mTPCPar = 0;
  mPadRow = 0;
  bUseDispList = false;  
  mRowCol.rgba(0.,0.,1.,0.07);
  mFocusCol.rgba(1.,0.,1.,0.57);
}

/**************************************************************************/
