// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ScreenDumper_H
#define GledCore_ScreenDumper_H

#include <Glasses/Operator.h>
#include <Glasses/PupilInfo.h>

class ScreenDumper : public Operator
{
  MAC_RNR_FRIENDS(ScreenDumper);

private:
  void _init();

protected:
  ZLink<PupilInfo> mPupil;        // X{gS} L{}

  Bool_t           bWaitSignal;   // X{GS} 7 Bool(-join=>1)
  Bool_t           bDumpImage;    // X{GS} 7 Bool()
  TString          mFileNameFmt;  // X{GS} 7 Textor()
  Int_t            mNTiles;       // X{GS} 7 Value(-range=>[1,4,1], -join=>1)
  Bool_t           bCopyToScreen; // X{GS} 7 Bool()

  Int_t            mDumpID;       // X{GS} 7 Value()

public:
  ScreenDumper(const Text_t* n="ScreenDumper", const Text_t* t=0) :
    Operator(n,t) { _init(); }

  void DumpScreen();

  virtual void Operate(Operator::Arg* op_arg);

#include "ScreenDumper.h7"
  ClassDef(ScreenDumper, 1);
}; // endclass ScreenDumper


#endif
