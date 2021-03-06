// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_RndSMorphCreator_H
#define Geom1_RndSMorphCreator_H

#include <Glasses/Operator.h>
#include <Glasses/ZNode.h>

#include <TRandom.h>

class RndSMorphCreator : public Operator {
  MAC_RNR_FRIENDS(RndSMorphCreator);

private:
  void _init();

protected:
  ZLink<ZNode>	mTarget;	// X{gS} L{}

  Bool_t	bReportID;	// X{gS} 7 Bool(-join=>1)
  Bool_t	bGetResult;	// X{gS} 7 Bool()

  TRandom	mRnd;		//!
  Double_t rnd(Double_t k=1, Double_t n=0);

public:
  RndSMorphCreator(const Text_t* n="RndSMorphCreator", const Text_t* t=0) : Operator(n,t) { _init(); }

  // virtuals
  virtual void Operate(Operator::Arg* op_arg);

#include "RndSMorphCreator.h7"
  ClassDef(RndSMorphCreator, 1);
}; // endclass RndSMorphCreator


#endif
