// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

//__________________________________________________________________________
// WSTube
//
//

#include "WSTube.h"
#include "WSTube.c7"
#include <Glasses/WSPoint.h>

#include <Glasses/ZHashList.h>
#include <Glasses/ZQueen.h>
#include <Glasses/Sphere.h>

#include <Gled/GThread.h>

#include <TSystem.h>

ClassImp(WSTube);

/**************************************************************************/

void WSTube::_init()
{
  mTransSource = TS_Links;

  bWeakConnect = true;
  mDefWidth    = 0.1;
  mDefSpread   = 0.02;
  mDefTension  = 1;

  mVecA.SetX(1);
  mVecB.SetX(1);

  mDefVelocity      = 0.2;
  mMinWaitTime      = 0.05;
  mConnectionStauts = CS_Disconnected;
  m_conn_cof = 0;
}

WSTube::~WSTube()
{
  delete m_conn_cof;
}

/**************************************************************************/

void WSTube::define_direction(ZTrans& t, TVector3& dr,
			      TLorentzVector& vec, TLorentzVector& sgm)
{
  TVector3 x;
  x += t.GetBaseVec(1) * (vec.X() + mRnd.Gaus(0, sgm.X()));
  x += t.GetBaseVec(2) * (vec.Y() + mRnd.Gaus(0, sgm.Y()));
  x += t.GetBaseVec(3) * (vec.Z() + mRnd.Gaus(0, sgm.Z()));
  x += dr              * (vec.T() + mRnd.Gaus(0, sgm.T()));

  x = x.Unit();
  TVector3 y = x.Orthogonal().Unit();
  t.SetBaseVec(1, x);
  t.SetBaseVec(2, y);
  t.SetBaseVec(3, x.Cross(y));
}

/**************************************************************************/

void WSTube::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  mRnd.SetSeed(mSaturnID);
  if(mTravelers == 0) {
    AList* l = new ZHashList("DependentLenses");
    l->SetElementFID(ZNode::FID());
    mQueen->CheckIn(l);
    SetTravelers(l);
  }
}

void WSTube::AdEndarkenment()
{
  PARENT_GLASS::AdEndarkenment();
  if(mTravelers != 0) {
    mQueen->RemoveLens(*mTravelers);
  }
}

/**************************************************************************/

void WSTube::connect(WSPoint*& ap, WSPoint*& bp)
{
  static const Exc_t _eh("WSTube::connect ");

  TString nameA, nameB;

  switch (mTransSource)
  {
    case TS_Links:
    {
      if(mNodeA == 0 || mNodeB == 0)
        throw(_eh + "node links A and B must be set.");

      ZNode* abcp = ZNode::FindCommonParent(mNodeA.get(), mNodeB.get());
      if(abcp == 0)
        throw(_eh + "nodes A and B have no common parent.");

      ZNode* top  = ZNode::FindCommonParent(this, abcp);
      if(top == 0)
        throw(_eh + "this WSTube does not have a common parent w/ nodes A and B.");


      auto_ptr<ZTrans> at( ZNode::BtoA(this, *mNodeA, top) );
      auto_ptr<ZTrans> bt( ZNode::BtoA(this, *mNodeB, top) );
      at->Unscale(); // Usually helpful; should be an option?
      bt->Unscale();
      mTransA = *at;
      mTransB = *bt;

      nameA = mNodeA->GetName();
      nameB = mNodeB->GetName();

      break;
    }
    case TS_Transes:
    {
      nameA = "<ext-a>";
      nameB = "<ext-b>";

      break;
    }
  }

  TVector3 dr( (mTransB.GetPos() - mTransA.GetPos()).Unit() ); // vector A -> B

  ap = new WSPoint("TubeStart", GForm("Tube %s->%s", nameA.Data(), nameB.Data()));
  ap->SetTrans(mTransA);
  define_direction(ap->ref_trans(), dr, mVecA, mSgmA);
  ap->SetW(mDefWidth/2);
  ap->SetScale(mDefSpread);
  ap->SetT(mDefTension);

  bp = new WSPoint("TubeEnd", GForm("Tube %s->%s", nameA.Data(), nameB.Data()));
  bp->SetTrans(mTransB);
  define_direction(bp->ref_trans(), dr, mVecB, mSgmB);
  bp->SetW(mDefWidth/2);
  bp->SetScale(-mDefSpread);
  bp->SetT(mDefTension);
}

void WSTube::disconnect()
{
  mTravelers->ClearList();
  m_traveler_list.clear();
  ClearList();
}

void WSTube::assert_disconnect(const Exc_t& eh)
{
  if(mConnectionStauts != CS_Disconnected) {
    if(bWeakConnect) {
      GLensWriteHolder wrlck(this);
      disconnect();
      mConnectionStauts = CS_Disconnected;
    } else {
      throw(eh + "not disconnected.");
    }
  }
}

void WSTube::conn_travel(WSPoint* p, Double_t t)
{
  // Expects t in [0..1]
  // Spread does not work.

  ZTrans& lcf = p->ref_trans();
  const Double_t t2 = t*t, t3 = t2*t;

  Double_t* Pnt = lcf.ArrT();
  Double_t* Axe = lcf.ArrX();

  for(Int_t i=0; i<3; i++) {
    const TMatrixDRow R( (*m_conn_cof)[i] );
    Pnt[i] = R[0] +   R[1]*t +   R[2]*t2 + R[3]*t3;
    Axe[i] = R[1] + 2*R[2]*t + 3*R[3]*t2;
  }
  {
    const TMatrixDRow R( (*m_conn_cof)[3] );
    p->mW = R[0] +   R[1]*t +   R[2]*t2 + R[3]*t3;
  }
  lcf.OrtoNorm3();
}

/**************************************************************************/

void WSTube::Connect()
{
  static const Exc_t _eh("WSTube::Connect ");

  assert_disconnect(_eh);

  WSPoint *ap, *bp;
  connect(ap, bp);
  // These should be via ZQueen::IncarnateWAttach for cluster stuff.
  { GLensWriteHolder wrlck(this);
    mQueen->CheckIn(ap); Add(ap);
    mQueen->CheckIn(bp); Add(bp);
  }

  mConnectionStauts = CS_Connected;
  mStampReqTring = Stamp(FID());
}

void WSTube::Disconnect()
{
  static const Exc_t _eh("WSTube::Disconnect ");

  if(mConnectionStauts != CS_Connected && !bWeakConnect)
    throw(_eh + "not connected.");

  disconnect();
  mConnectionStauts = CS_Disconnected;
  Stamp(FID());
}

void WSTube::AnimateConnect(Float_t velocity)
{
  static const Exc_t _eh("WSTube::AnimatedConnect ");

  assert_disconnect(_eh);

  WSPoint *ap, *bp;
  connect(ap, bp);
  Bool_t rnrself_state = bRnrSelf;
  bRnrSelf = false;
  // These should be via ZQueen::IncarnateWAttach for cluster stuff.
  { GLensWriteHolder wrlck(this);
    mQueen->CheckIn(ap); Add(ap);
    mQueen->CheckIn(bp); Add(bp);
  }

  ap->Coff(bp);
  m_conn_cof = new TMatrixD(ap->RefCoffs());
  bp->SetTrans(ap->RefTrans());

  if(velocity == 0) velocity = mDefVelocity;
  m_conn_time = 0;
  m_conn_vel  = velocity;

  bRnrSelf = rnrself_state;
  mConnectionStauts = CS_Connecting;
  Stamp(FID());
}

void WSTube::AnimateDisconnect(Float_t velocity, Bool_t delay_p)
{
  // If delay_p==true resend mir to self if connecting.

  static const Exc_t _eh("WSTube::AnimateDisconnect ");

  if(mConnectionStauts == CS_Connecting && delay_p) {
    auto_ptr<ZMIR> redo( S_AnimateDisconnect(velocity, delay_p) );
    GTime time(GTime::I_Now);
    // Don't know anything about time scale ... so take 1 seconf.
    time += 1.0;
    mSaturn->DelayedShootMIR(redo, time);
    return;
  }

  if(mConnectionStauts != CS_Connected)
    throw(_eh + "not connected.");

  WSPoint *ap, *bp;
  connect(ap, bp);
  ap->Coff(bp);
  m_conn_cof = new TMatrixD(m_first_point->RefCoffs());

  if(velocity == 0) velocity = mDefVelocity;
  m_conn_time = Length();
  m_conn_vel  = -velocity;

  mConnectionStauts = CS_Disconnecting;

  Stamp(FID());
}

/**************************************************************************/

void WSTube::MakeTraveler(Float_t velocity, Float_t wait_time)
{
  static const Exc_t _eh("WSTube::MakeTraveler ");

  if(mConnectionStauts != CS_Connected && mConnectionStauts != CS_Connecting)
    throw(_eh + "not connected or connecting.");

  if(velocity == 0)            velocity  = mDefVelocity;
  if(wait_time < mMinWaitTime) wait_time = mMinWaitTime;

  Float_t len      = Length();
  Float_t init_pos = (velocity > 0) ? 0 : len;

  Sphere* s = new Sphere("Observator");
  TransAtTime(s->ref_trans(), init_pos, true);
  s->SetUseScale(true);
  s->SetSx(2*mDefWidth); s->SetSy(mDefWidth); s->SetSz(mDefWidth);
  s->SetColor(0.2+0.8*mRnd.Rndm(), 0.2+0.8*mRnd.Rndm(), 0.2+0.8*mRnd.Rndm());
  mQueen->CheckIn(s);
  s->SetParent(this);

  list<Traveler>::iterator list_pos = m_traveler_list.end();
  if( ! m_traveler_list.empty()) {
    if(velocity > 0) {
      if(m_traveler_list.front().fPosition < wait_time &&
	 m_traveler_list.front().fVelocity > 0)
	{
	  init_pos = m_traveler_list.front().fPosition - wait_time;
	}
      list_pos = m_traveler_list.begin();
    } else {
      if(m_traveler_list.back().fPosition > len - wait_time &&
	 m_traveler_list.back().fVelocity < 0)
	{
	  init_pos = m_traveler_list.back().fPosition + wait_time;
	}
    }
  }
  m_traveler_list.insert(list_pos, Traveler(s, init_pos , velocity));
}

void WSTube::TimeTick(Double_t t, Double_t dt)
{
  if( mConnectionStauts == CS_Connected ||
     (mConnectionStauts == CS_Disconnecting && ! m_traveler_list.empty())) {

    PARENT_GLASS::TimeTick(t, dt);

    GLensWriteHolder wrlck(this);
    Float_t len = Length();
    list<Traveler>::iterator i = m_traveler_list.begin();
    while(i != m_traveler_list.end()) {
      Float_t step  = i->fVelocity*dt;
      i->fPosition += step;
      if(i->fShown == false && i->fPosition >= 0 && i->fPosition <= len) {
	mTravelers->Add(i->fNode);
	i->fShown = true;
      }
      if((i->fVelocity < 0 && i->fPosition < 0) ||
	 (i->fVelocity > 0 && i->fPosition > len))
	{
	  list<Traveler>::iterator j = i--;
	  mTravelers->RemoveAll(j->fNode);
	  m_traveler_list.erase(j);
	  break;
	}
      {
	GLensWriteHolder node_wrlck(i->fNode);
	TransAtTime(i->fNode->ref_trans(), i->fPosition, false);
	i->fNode->StampReqTrans();
      }
      ++i;
    }

  }
  else if(mConnectionStauts == CS_Connecting ||
	  mConnectionStauts == CS_Disconnecting) {

    GLensWriteHolder wrlck(this);
    Float_t len  = Length();
    Float_t step = m_conn_vel*dt;
    Bool_t done  = false;
    m_conn_time += step;

    if(m_conn_vel < 0 && m_conn_time  < 0) {
      disconnect();
      mConnectionStauts = CS_Disconnected;
      done = true;
    } else {
      if(m_conn_vel > 0 && m_conn_time > len) {
	m_conn_time       = len;
	mConnectionStauts = CS_Connected;
      done = true;
      }
      conn_travel(m_last_point, m_conn_time/len);
      m_last_point->Stamp(WSPoint::FID());
    }
    if(done) {
      delete m_conn_cof; m_conn_cof = 0;
      Stamp(FID());
    }
  }
}
