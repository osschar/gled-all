// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GledMonitor.h"
#include <Glasses/SaturnInfo.h>
#include "GledMonitor.c7"

#include <Gled/GledMirDefs.h>

#include <TMath.h>
#include <TSystem.h>

//__________________________________________________________________________
//
// Operator that collects local cpu & mem information and updates state
// of appropriate SaturnInfo.
// If bFillHistos is true fills load average histograms. To access them,
// type 'new TBrowser' and navigate: 'ROOT files'-><monitor>->LAvg, then
// double-click on histogram icon.

ClassImp(GledMonitor);

/**************************************************************************/

void GledMonitor::_init()
{
  mCpuSampleTime = 200; // Sampling time when getting CpuInfo [ms].
}

/**************************************************************************/

void GledMonitor::AdEnlightenment()
{
  Operator::AdEnlightenment();
  InitHistoGroups();
}

void GledMonitor::Operate(Operator::Arg* op_arg)
{
  Operator::PreOperate(op_arg);

  CpuInfo_t cpu; gSystem->GetCpuInfo(&cpu, mCpuSampleTime);
  MemInfo_t mem; gSystem->GetMemInfo(&mem);

  SaturnInfo* si = mSaturn->GetSaturnInfo();

  // !!!! here should realy send cpu/mem ... need setters in SaturnInfo.
  // !!!! also, CpuInfo_t should be fixed to include 'nice' usage.

  SP_MIR(si, SetMFree, mem.fMemFree);
  SP_MIR(si, SetSFree, mem.fSwapFree);

  SP_MIR(si, SetLAvg1,  cpu.fLoad1m);
  SP_MIR(si, SetLAvg5,  cpu.fLoad5m);
  SP_MIR(si, SetLAvg15, cpu.fLoad15m);

  SP_MIR(si, SetCU_Total, 0.01*TMath::Nint(100.0*cpu.fTotal));
  SP_MIR(si, SetCU_User,  0.01*TMath::Nint(100.0*cpu.fUser));
  SP_MIR(si, SetCU_Nice,  0);
  SP_MIR(si, SetCU_Sys,   0.01*TMath::Nint(100.0*cpu.fSys));
  SP_MIR(si, SetCU_Idle,  0.01*TMath::Nint(100.0*cpu.fIdle));

  if(bFillHistos) {
    h1LAvg1 ->Fill(cpu.fLoad1m);
    h1LAvg5 ->Fill(cpu.fLoad5m);
    h1LAvg15->Fill(cpu.fLoad15m);
  }

  Operator::PostOperate(op_arg);
}
