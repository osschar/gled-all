// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// AEVEventBatch
//
//

#include "AEVEventBatch.h"
#include "AEVEventBatch.c7"

#include <TMath.h>

ClassImp(AEVEventBatch)

/**************************************************************************/

void AEVEventBatch::_init()
{
  mNWorkers   = 0;
  mDataSizeMB = mDataDoneMB = 0;

  mTPerEvAvg = 1;
  mTPerEvSgm = 0.05;
  mFracFail  = 0.05;

  mSimEvs    = 10;
  mProcAvg   = 2;

  mRnrMode   = RM_Bar;

  mHDepth = 1;
  mColAll.rgba (0.1, 0.1, 0.8);
  mColOK.rgba  (0,   0.8, 0.2);
  mColFail.rgba(0.8,   0, 0.2);
  mColProc.rgba(0.1, 0.6, 0.6);

  // Enforce usage of scale.
  bUseScale = true;
}

void AEVEventBatch::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  mRnd.SetSeed(mSaturnID);
}

/**************************************************************************/

void AEVEventBatch::Reinit()
{
  mEvState.Reinit();
  Stamp(FID());
}

void AEVEventBatch::Reinit(Int_t n_events)
{
  mEvState.Reinit(n_events);
  Stamp(FID());
}

/**************************************************************************/

void AEVEventBatch::FakeInit()
{
  Int_t nall = TMath::Nint( mSimEvs*(0.75 + 0.5*mRnd.Rndm()) );
  // Int_t nx   = TMath::Nint( nall * (2*mFracFail*mRnd.Rndm()) );
  mEvState.SetNAll(nall);
  mEvState.SetNFail(0);
  mEvState.SetNOK(0);
  mEvState.SetNProc(0); // mProcAvg*(0.75 + 0.25*mRnd.Rndm()));
  Stamp(FID());
}

void AEVEventBatch::FakeProc()
{
  SEvTaskState& es( mEvState );

  Int_t nleft = es.GetNLeft();
  Int_t nnew  = TMath::Min(TMath::Nint(mProcAvg*(0.75 + 0.5*mRnd.Rndm())),
                           nleft);
  Int_t nfin  = TMath::Min(TMath::Nint(mProcAvg*(0.75 + 0.5*mRnd.Rndm())),
                           es.GetNProc());
  Int_t nfail = TMath::Min((mRnd.Rndm() < mFracFail ? 1 : 0), nfin);

  es.SetNOK  (es.GetNOK()   + nfin - nfail);
  es.SetNFail(es.GetNFail() + nfail);
  Int_t nproc = es.GetNProc() - nfin + nnew;
  if(nproc > es.GetNToDo())
    nproc = es.GetNToDo();
  if(nproc < 0)
    nproc = 0;
  es.SetNProc(nproc);

  Stamp(FID());
}

/**************************************************************************/
