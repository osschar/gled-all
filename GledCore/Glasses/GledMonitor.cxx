// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// GledMonitor
//
// Operator that collects local cpu & mem information and updates state
// of appropriate SaturnInfo.
// If bFillHistos is true fills load average histograms. To access them,
// type 'new TBrowser' and navigate: 'ROOT files'-><monitor>->LAvg, then
// double-click on histogram icon.

#include "GledMonitor.h"
#include <Glasses/SaturnInfo.h>
#include <Gled/GledMirDefs.h>

#include <TSystem.h>



ClassImp(GledMonitor)

/**************************************************************************/

void GledMonitor::_init()
{
  FILE* p = gSystem->OpenPipe("GledNodeReport.pl cpureport", "r");
  if(p != 0) {
    fscanf(p, "%ud %ud %ud %ud", &m_j[0], &m_j[1], &m_j[2], &m_j[3]);
    gSystem->ClosePipe(p);
  } else {
    m_j[0] = m_j[1] = m_j[2] = m_j[3] = 0;
  }
}

/**************************************************************************/

void GledMonitor::AdEnlightenment()
{
  Operator::AdEnlightenment();
  InitHistoGroups();
}

void GledMonitor::Operate(Operator::Arg* op_arg) throw(Operator::Exception)
{
  Operator::PreOperate(op_arg);

  UShort_t fmem=0, fswp=0;
  Float_t l1=0, l5=0, l15=0;
  Float_t ctot=0, cusr=0, cnice=0, csys=0;

  UInt_t j[4];

  FILE* p = gSystem->OpenPipe("GledNodeReport.pl memfree cpureport", "r");
  if(p != 0) {
    fscanf(p, "%hu %hu", &fmem, &fswp);
    fscanf(p, "%u %u %u %u", &j[0], &j[1], &j[2], &j[3]);
    fscanf(p, "%f %f %f", &l1, &l5, &l15);
    gSystem->ClosePipe(p);

    UInt_t dt = 0; for(int i=0; i<4; ++i) dt += j[i] - m_j[i];
    
    ctot  = (Float_t)(j[0]-m_j[0] + j[1]-m_j[1] + j[2]-m_j[2])/dt;
    ctot  = int(1000*ctot)/1000.0;
    cusr  = (Float_t)(j[0]-m_j[0])/dt;
    cusr  = int(1000*cusr)/1000.0;
    cnice = (Float_t)(j[1]-m_j[1])/dt;
    cnice = int(1000*cnice)/1000.0;
    csys  = (Float_t)(j[2]-m_j[2])/dt;
    csys  = int(1000*csys)/1000.0;
  } else {
    j[0] = j[1] = j[2] = j[3] = 0;
  }

  SaturnInfo* si = mSaturn->GetSaturnInfo();

  SP_MIR(si, SetMFree, fmem);
  SP_MIR(si, SetSFree, fswp);

  SP_MIR(si, SetLAvg1, l1);
  SP_MIR(si, SetLAvg5, l5);
  SP_MIR(si, SetLAvg15, l15);

  SP_MIR(si, SetCU_Total, ctot);
  SP_MIR(si, SetCU_User,  cusr);
  SP_MIR(si, SetCU_Nice,  cnice);
  SP_MIR(si, SetCU_Sys,   csys);

  for(int i=0; i<4; ++i) m_j[i] = j[i];

  if(bFillHistos) {
    h1LAvg1->Fill(l1);
    h1LAvg5->Fill(l5);
    h1LAvg15->Fill(l15);
  }

  Operator::PostOperate(op_arg);
}

/**************************************************************************/

#include "GledMonitor.c7"
