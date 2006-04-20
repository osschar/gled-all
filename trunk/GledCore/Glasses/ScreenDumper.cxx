// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
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
  mPupil       = 0;
  mFileNameFmt = "screendumper/img%04d.tga";

  mNTiles   = 1;
  bWaitDump = false;

  mDumpID = 0;
}

/**************************************************************************/

void ScreenDumper::DumpScreen()
{
  if(mPupil != 0) {
    TString fname(GForm(mFileNameFmt.Data(), mDumpID++));
    if(bWaitDump)
      mPupil->DumpImageWaitSignal(fname, mNTiles);
    else
      mPupil->EmitDumpImageRay(fname, mNTiles);
  }
  Stamp(FID());
}

void ScreenDumper::Operate(Operator::Arg* op_arg) throw(Exception)
{
  PreOperate(op_arg);
  DumpScreen();
  PostOperate(op_arg);
}

/**************************************************************************/
