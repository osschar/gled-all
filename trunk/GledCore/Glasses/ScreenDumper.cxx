// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// ScreenDumper
//
//

#include "ScreenDumper.h"

#include "ScreenDumper.c7"

ClassImp(ScreenDumper)

/**************************************************************************/

void ScreenDumper::_init()
{
  bWaitSignal   = false;
  bDumpImage    = false;
  mFileNameFmt  = "screendumper/img%04d";
  mNTiles       = 1;
  bCopyToScreen = false;

  mDumpID = 0;
}

/**************************************************************************/

void ScreenDumper::DumpScreen()
{
  if(mPupil != 0)
  {
    if (bDumpImage)
    {
      TString fname(GForm(mFileNameFmt.Data(), mDumpID++));
      if (bWaitSignal)
        mPupil->DumpImageWaitSignal(fname, mNTiles, bCopyToScreen);
      else
        mPupil->DumpImage(fname, mNTiles, bCopyToScreen);
    }
    else
    {
      if (bWaitSignal)
        mPupil->RedrawWaitSignal();
      else
      mPupil->Redraw();
    }
    Stamp(FID());
  }
}

void ScreenDumper::Operate(Operator::Arg* op_arg) throw(Exception)
{
  PreOperate(op_arg);
  DumpScreen();
  PostOperate(op_arg);
}

/**************************************************************************/
