// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ScreenDumper_H
#define GledCore_ScreenDumper_H

#include <Glasses/Operator.h>
#include <Glasses/PupilInfo.h>

class ScreenDumper : public Operator {
  MAC_RNR_FRIENDS(ScreenDumper);

private:
  void _init();

protected:
  ZLink<PupilInfo>	mPupil; 	// X{gS} L{}
  TString	mFileNameFmt;	// X{GS} 7 Textor()
  
public:
  ScreenDumper(const Text_t* n="ScreenDumper", const Text_t* t=0) : Operator(n,t) { _init(); }

  virtual void Operate(Operator::Arg* op_arg)     throw(Operator::Exception);

#include "ScreenDumper.h7"
  ClassDef(ScreenDumper, 1)
}; // endclass ScreenDumper


#endif
