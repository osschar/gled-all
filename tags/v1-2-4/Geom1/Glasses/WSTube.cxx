// $Header$

// Copyright (C) 1999-2004, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// WSTube
//
//

#include "WSTube.h"
#include "WSTube.c7"
#include <Glasses/WSPoint.h>

#include <Glasses/ZQueen.h>
#include <Glasses/Sphere.h>

#include <Gled/GThread.h>

#include <TSystem.h>

ClassImp(WSTube)

/**************************************************************************/

void WSTube::_init()
{
  // WSSeed overrides
  bRenormLen = true; mLength = 1;

  // *** Set all links to 0 ***
  mNodeA = mNodeB = 0;
  mLenses = 0;

  mDefWidth   = 0.1;
  mDefSpread  = 0.02;
  mDefTension = 1;

  mVecA.SetX(1);
  mVecB.SetX(1);

  mSleepMS = 50;
  mInitDt  = 0.008;
}

/**************************************************************************/

void WSTube::define_direction(ZTrans& t, TVector3& dr,
			      TLorentzVector& vec, TLorentzVector& sgm)
{
  TVector3 x;
  x += t.GetBaseVec3(1) * (vec.X() + mRnd.Gaus(0, sgm.X()));
  x += t.GetBaseVec3(2) * (vec.Y() + mRnd.Gaus(0, sgm.Y()));
  x += t.GetBaseVec3(3) * (vec.Z() + mRnd.Gaus(0, sgm.Z()));
  x += dr               * (vec.T() + mRnd.Gaus(0, sgm.T()));

  x = x.Unit();
  TVector3 y = x.Orthogonal().Unit();
  t.SetBaseVec3(1, x);
  t.SetBaseVec3(2, y);
  t.SetBaseVec3(3, x.Cross(y));
}

/**************************************************************************/

void WSTube::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  mRnd.SetSeed(mSaturnID);
  if(mLenses == 0) {
    ZList* l = new ZList("DependentLenses");
    l->SetElementFID(ZNode::FID());
    mQueen->CheckIn(l);
    SetLenses(l);
  }
}

void WSTube::AdEndarkenment()
{
  PARENT_GLASS::AdEndarkenment();
  if(mLenses != 0) {
    mQueen->RemoveLens(mLenses);
  }
}

/**************************************************************************/

void WSTube::Connect()
{
  static const string _eh("WSTube::Connect2 ");

  if(mNodeA == 0 || mNodeB == 0)
    throw(_eh + "node links A and B must be set.");

  ZNode* abcp = ZNode::FindCommonParent(mNodeA, mNodeB);
  if(abcp == 0)
    throw(_eh + "nodes A and B have no common parent.");

  ZNode* top  = ZNode::FindCommonParent(this, abcp);
  if(top == 0)
    throw(_eh + "this WSTube does not have a common parent w/ nodes A and B.");

  ZList::ClearList();

  auto_ptr<ZTrans> at( ZNode::BtoA(this, mNodeA, top) );
  auto_ptr<ZTrans> bt( ZNode::BtoA(this, mNodeB, top) );
  TVector3 dr( (bt->GetPosVec3() - at->GetPosVec3()).Unit() ); // vector A -> B
  // ZTrans x(*at); x.Invert(); x *= *bt;
  

  WSPoint* ap = new WSPoint("TubeStart", GForm("Tube %s->%s", mNodeA->GetName(), mNodeB->GetName()));
  define_direction(*at, dr, mVecA, mSgmA);
  ap->SetTrans(*at);
  ap->SetW(mDefWidth/2);
  ap->SetS(mDefSpread);
  ap->SetT(mDefTension);

  WSPoint* bp = new WSPoint("TubeEnd", GForm("Tube %s->%s", mNodeA->GetName(), mNodeB->GetName()));
  define_direction(*bt, dr, mVecB, mSgmB);
  bp->SetTrans(*bt);
  bp->SetW(mDefWidth/2);
  bp->SetS(-mDefSpread);
  bp->SetT(mDefTension);

  // These should be via ZQueen::IncarnateWAttach for cluster stuff.
  mQueen->WriteLock();
  mQueen->CheckIn(ap);
  mQueen->CheckIn(bp);
  mQueen->WriteUnlock();

  Add(ap);
  Add(bp);

  mStampReqTring = Stamp(FID());
}

void WSTube::AnimatedConnect()
{
  static const string _eh("WSTube::AnimatedConnect ");

  WSPoint* B;
  ZTrans   transB;
  {
    GLensWriteHolder wrlck(this);
    Connect();
    B = dynamic_cast<WSPoint*>(Last());
    if(B == 0)
      throw(_eh + "last point not reachable (by trivial algo).");
    auto_ptr<ZTrans> trans ( init_slide(0) );
    transB = B->RefTrans();
    B->SetTrans(*trans);
  }

  Float_t time = 0, dt = mInitDt, max_t = mLength;
  bool done = false;
  while(!done) {
    gSystem->Sleep(mSleepMS);

    time += dt;
    if(time > max_t) {
      time = max_t;
      done = true;
    }

    GLensWriteHolder wrlck(this);
    {
      GLensWriteHolder B_wrlck(B);
      ZTrans tx(B->RefTrans());
      B->SetTrans(transB);
      TransAtTime(tx, time, false);
      B->SetTrans(tx);
    }
    mStampReqTring = Stamp(FID());
  }
}

/**************************************************************************/

void WSTube::TravelAtoB()
{
  Travel(mInitDt, mSleepMS, true);
}

void WSTube::TravelBtoA()
{
  Travel(mInitDt, mSleepMS, false);
}

void WSTube::Travel(Float_t abs_dt, UInt_t sleep_ms, Bool_t AtoB)
{
  Sphere* s = new Sphere("Observator");
  ZTrans trans;
  {
    GLensWriteHolder wrlck(this);
    ZTrans* tt = init_slide(0);
    trans = *tt;
    delete tt;
    if(AtoB == false) {
      TransAtTime(trans, mLength, false);
      TVector3 xcy = trans.GetBaseVec3(1);
      xcy = xcy.Cross(trans.GetBaseVec3(2));
      TVector3 z(trans.GetBaseVec3(3));
      if(xcy.Dot(z) < 0) {
	trans.SetBaseVec3(3, -z);
      }
    }
    s->SetTrans(trans);
    s->SetUseScale(true);
    s->SetSx(2*mDefWidth); s->SetSy(mDefWidth); s->SetSz(mDefWidth); 
    s->SetColor(0.2+0.8*mRnd.Rndm(), 0.2+0.8*mRnd.Rndm(), 0.2+0.8*mRnd.Rndm());
  }

  {
    GLensWriteHolder q_wrlck(mQueen);
    mQueen->CheckIn(s);   // try-catch !!!!
  }

  {
    GLensWriteHolder s_wrlck(s);
    s->SetParent(this);
    // s->IncRefCount(this); // try-catch
  }

  // This whole thing should be split into two functions for
  // proper operation in a cluster context.
  // 1. Instantiation at the Sun, Broadcast of MIR to call stepper in a
  //    dedicated thread. (declared as X{E}, require MIR)
  // 2. Stepper itself (declared as X{ED})


  {
    GLensWriteHolder lenses_wrlck(mLenses);
    mLenses->Add(s);
  }
  Float_t
    time  = AtoB ? 0 : mLength,
    dt    = AtoB ? abs_dt : -abs_dt,
    max_t = AtoB ? mLength : 0;

  while(1) {
    gSystem->Sleep(sleep_ms);
    time += dt;
    if(time*(AtoB ? 1 : -1) > max_t) {
      {
	GLensWriteHolder s_wrlck(s);
	s->SetParent(0);
      }
      {
	GLensWriteHolder wrlck(mLenses);
	// s->DecRefCount(this);
	mLenses->Remove(s);
	// mQueen->RemoveLens(s);
      }
      break;
    }
    {
      GLensWriteHolder wrlck(this);
      TransAtTime(trans, time, false);
    }
    {
      GLensWriteHolder wrlck(s);
      s->SetTrans(trans);
    }
  }
}
