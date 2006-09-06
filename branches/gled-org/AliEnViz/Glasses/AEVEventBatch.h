// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVEventBatch_H
#define AliEnViz_AEVEventBatch_H

#include <Glasses/ZNode.h>
#include <Stones/SEvTaskState.h>
#include <Stones/ZColor.h>

#include <TRandom.h>

class AEVEventBatch : public ZNode {
  MAC_RNR_FRIENDS(AEVEventBatch);
  friend class AEVDistAnRep;

public:
  enum RnrMode_e { RM_Combined, RM_Bar };

private:
  void _init();

protected:
  Int_t		mNWorkers;	// X{GS}  7 ValOut(-width=>4)
  Double_t	mDataSizeMB;	// X{GS}  7 ValOut(-width=>8, -join=>1)
  Double_t	mDataDoneMB;	// X{GS}  7 ValOut(-width=>8)

  SEvTaskState	mEvState;	// X{GSR} 7 StoneOutput(Fmt=>"%c: %d [ %d | %d | %d ]", Args=>[State, NAll, NOK, NFail, NProc])

  Float_t	mTPerEvAvg;	// X{GS}  7 Value(-range=>[0,1e6,1,1000], join=>1);
  Float_t	mTPerEvSgm;	// X{GS}  7 Value(-range=>[0,1e3,1,1000]);

  Float_t	mFracFail;	// X{GS}  7 Value(-range=>[0,1,1,1000]);

  Int_t		mSimEvs;	// X{GS}  7 Value(-range=>[0,1e3,1,1], -join=>1);
  Int_t		mProcAvg;	// X{GS}  7 Value(-range=>[0,1e3,1,1]);

  RnrMode_e	mRnrMode;	// X{GS}  7 PhonyEnum()

  Float_t	mHDepth;	// X{GS}  7 Value(-range=>[0,10,1,100]);
  ZColor	mColAll;	// X{GSP} 7 ColorButt(-join=>1)
  ZColor	mColOK;		// X{GSP} 7 ColorButt()
  ZColor	mColFail;	// X{GSP} 7 ColorButt(-join=>1)
  ZColor	mColProc;	// X{GSP} 7 ColorButt()

  TRandom	mRnd;

public:
  AEVEventBatch(const Text_t* n="AEVEventBatch", const Text_t* t=0) : 
    ZNode(n,t) { _init(); }

  virtual void AdEnlightenment();

  void Reinit();
  void Reinit(Int_t n_events);

  void FakeInit(); // X{E} 7 MButt(-join=>1)
  void FakeProc(); // X{E} 7 MButt()

  Bool_t IsDone() {
    return mEvState.GetNProc() == 0 && mEvState.GetNToDo() == 0;
  }

  void IncProcessing(Int_t delta=1) {
    if(mEvState.GetState() == 'W') mEvState.SetState('R');
    mEvState.IncNProc(delta);
    Stamp(FID());
  }
  void DecProcessing(Int_t delta=-1) {
    mEvState.IncNProc(delta);
    Stamp(FID());
  }

  void Finalize() {
    if(mEvState.GetState() != 'F') {
      mEvState.SetNProc(0);
      mEvState.IncNFail(mEvState.GetNLeft());
      mEvState.SetState('F');
    }
    Stamp(FID());
  }

#include "AEVEventBatch.h7"
  ClassDef(AEVEventBatch, 1)
}; // endclass AEVEventBatch


#endif
