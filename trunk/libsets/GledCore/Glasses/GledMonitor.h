// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Gled_GledMonitor_H
#define Gled_GledMonitor_H

#include <Glasses/Operator.h>
#include <Stones/ZHisto.h>

#include <TH1F.h>

class GledMonitor : public Operator, public ZHisto
{
  MAC_RNR_FRIENDS(GledMonitor);

private:
  UInt_t	m_j[4];	//!

  void _init();
  void _get_jiffies(UInt_t* j);

protected:
  Bool_t	bFillHistos;	// X{GS} 7 Bool()

  // 7777 InstallHandler(GLED::Histo);

  TH1F*		h1LAvg1;  // X{gs} H7_LAvg("%m","%m for %c", 100, 0, 10)
  TH1F*		h1LAvg5;  // X{gs} H7_LAvg("%m","%m for %c", 100, 0, 10)
  TH1F*		h1LAvg15; // X{gs} H7_LAvg("%m","%m for %c", 100, 0, 10)

public:
  GledMonitor(const Text_t* n="GledMonitor", const Text_t* t=0) :
    Operator(n,t) { _init(); }

  virtual void AdEnlightenment();
  virtual void Operate(Operator::Arg* op_arg);

#include "GledMonitor.h7"
  ClassDef(GledMonitor, 1);
}; // endclass GledMonitor


#endif
