// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// RndSMorphCreator
//
//

#include "RndSMorphCreator.h"
#include "RndSMorphCreator.c7"
#include "SMorph.h"
#include <Glasses/ZQueen.h>

#include <Gled/GledNS.h>

ClassImp(RndSMorphCreator)

/**************************************************************************/

void RndSMorphCreator::_init()
{
  // !!!! Set all links to 0 !!!!
  mTarget = 0;
  bReportID  = true;
  bGetResult = true;
  mRnd.SetSeed(0);
}

/**************************************************************************/

inline
Double_t RndSMorphCreator::rnd(Double_t k, Double_t n)
{
  return k*mRnd.Rndm() + n;
}

/**************************************************************************/

void RndSMorphCreator::Operate(Operator::Arg* op_arg) throw(Operator::Exception)
{
  static const string _eh("Mover::Operate ");

  Operator::PreOperate(op_arg);
  if(mTarget) {
    SMorph m(GForm("Random SMorph %d", int(rnd(1e6,1))));
    m.Set3Pos(rnd(20,-10), rnd(20,-10), rnd(20,-10));
    m.SetUseScale(1);
    m.SetScales(rnd(1.5,0.5), rnd(1.5,0.5), rnd(1.5,0.5));
    m.SetTLevel((int)rnd(20,3)); m.SetPLevel((int)rnd(20,3));
    m.SetColor(rnd(0.6,0.4), rnd(0.6,0.4), rnd(0.6,0.4));

    FID_t fid = ZNode::FID();
    MID_t mid = ZNode::Mid_Add(); // 
    auto_ptr<ZMIR> mir
      (mTarget->GetQueen()->S_IncarnateWAttach(mTarget, 0, fid.lid, fid.cid, mid));
    GledNS::StreamLens(*mir, &m);

    if(bGetResult) {
      auto_ptr<ZMIR_RR> res( mSaturn->ShootMIRWaitResult(mir) );
      if(res->HasException()) {
	cout << _eh << "got exception: " << res->Exception.Data() << endl;
      
      } 
      if(res->HasResult()) {
	ID_t id; *res >> id;
	if(bReportID)
	  printf("%sgot id %u, at %p\n", _eh.c_str(), id, mSaturn->DemangleID(id));
      }
    } else {
      mSaturn->ShootMIR(mir);
    }
  }
  Operator::PostOperate(op_arg);
}

/**************************************************************************/
