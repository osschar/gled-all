// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SolarSystem.h"
#include "SolarSystem.c7"

#include "CosmicBall.h"

#include <Gled/GThread.h>

#include <TVectorT.h>
#include <TRandom.h>
#include <TMath.h>
#include <TSystem.h>


// SolarSystem

//__________________________________________________________________________
//
// Simple simulation of (unrealistic) solar system.
//
//

ClassImp(SolarSystem);

/**************************************************************************/

void SolarSystem::_init()
{
  mTime    = 0;
  mTimeFac = 1;

  mStarMass   = 0;
  mBallKappa  = 1e-6;

  mPlanetMinR = 20;
  mPlanetMaxR = 40;
  mOrbitMinR  = 150;
  mOrbitMaxR  = 1000;
  mMaxTheta   = 20;
  mMaxEcc     = 0.1;

  mIntegratorThread = 0;
}

SolarSystem::~SolarSystem()
{
  if (mIntegratorThread)
    mIntegratorThread->Cancel();
}

/**************************************************************************/

void SolarSystem::AdEnlightenment()
{
  PARENT_GLASS::AdEnlightenment();
  if (mBalls == 0) {
    ZVector* l = new ZVector("Balls", GForm("Balls of SolarSystem %s", GetName()));
    l->SetElementFID(CosmicBall::FID());
    mQueen->CheckIn(l);
    SetBalls(l);
  }
  if (mODECrawler == 0) {
    ODECrawler* l = new ODECrawler("ODECrawler", GForm("ODECrawler of SolarSystem %s", GetName()));
    mQueen->CheckIn(l);
    SetODECrawler(l);
    mODECrawler->SetODEMaster(this);
  }
}

/**************************************************************************/

UInt_t SolarSystem::ODEOrder()
{
  static const string _eh("SolarSystem::ODEOrder ");

  assert_balls(_eh);
  return 6 * mBalls->Size();
}

void SolarSystem::ODEDerivatives(const Double_t x, const TVectorD& y, TVectorD& d)
{
  // Stella moves not.
  d(0) = d(1) = d(2) = d(3) = d(4) = d(5) = 0;

  // For the rest, just take Stella at origin for now.
  TVector3 delta;
  Double_t rfac;
  Int_t    max_i = ODEOrder();
  for (Int_t i = 6; i < max_i; i += 6)
  {
    d(i)   = y(i+3);
    d(i+1) = y(i+4);
    d(i+2) = y(i+5);

    delta.SetXYZ(-y(i), -y(i+1), -y(i+2));
    rfac = delta.Mag2();
    rfac = mStarMass / (rfac*TMath::Sqrt(rfac));
    d(i+3) = mBallKappa * delta.x() * rfac;
    d(i+4) = mBallKappa * delta.y() * rfac;
    d(i+5) = mBallKappa * delta.z() * rfac;
  }
}

void SolarSystem::ODEStart(TVectorD& v, Double_t& x1, Double_t& x2)
{
  Int_t i = 0;
  Stepper<CosmicBall> stepper(*mBalls);
  while (stepper.step())
  {
    const Double_t* P = stepper->RefTrans().ArrT();
    const TVector3& V = stepper->RefV();

    v(i++) = P[0];  v(i++) = P[1];  v(i++) = P[2];
    v(i++) = V.x(); v(i++) = V.y(); v(i++) = V.z();
  }

  assert(i == (Int_t) ODEOrder());
}

/**************************************************************************/

void SolarSystem::TimeTick(Double_t t, Double_t dt)
{
  GLensWriteHolder wrlck(this);

  Double_t new_time = mTime + mTimeFac*dt;
  ODEStorageD* S = get_storage();
  if (new_time > S->GetMaxXStored())
  {
    SetTime(S->GetMaxXStored());
    mODECrawler->AdvanceXLimits();
    mODECrawler->Crawl();
  }
  SetTime(new_time);
}

/**************************************************************************/

void SolarSystem::StartStorageManager()
{
  // not finished.
  //
  // check if already running
  // initialize crawler
  // crawl - forewer

  static const Exc_t _eh("SolarSystem::StartStorageManager ");

  assert_MIR_presence(_eh, MC_IsBeam);

  if (mIntegratorThread)
    throw (_eh + "already running.");

  mIntegratorThread = GThread::TSDSelf();

  while (1)
  {
    printf("Fujfufjf\n");
    gSystem->Sleep(1000);
  }
}

void SolarSystem::StopStorageManager()
{
  // not finished

  static const Exc_t _eh("SolarSystem::StopStorageManager ");

  if (mIntegratorThread == 0)
    throw (_eh + "not running.");

  mIntegratorThread->Cancel();
  mIntegratorThread = 0;
}

/**************************************************************************/

void SolarSystem::SetTime(Double_t t)
{
  // Place balls into position for time t.
  // Linear extrapolation between to stored points is made.

  static const Exc_t _eh("SolarSystem::SetTime ");

  ODEStorageD*    S = get_storage();
  const Int_t     N = S->Size();
  const Double_t* T = S->GetXArr();

  Int_t i0;
  if (t <= T[0])
  {
    if (t < T[0])
      ISwarn(_eh + GForm("given time %f below minimum %f. Using min", t, T[0]));
    i0 = 0;
    t  = T[0];
  }
  else if (t >= T[N-1])
  {
    if (t > T[N-1])
      ISwarn(_eh + GForm("given time %f above maximum %f. Using max", t, T[N-1]));
    i0 = N - 2;
    t  = T[N-1];
  }
  else
  {
    i0 = TMath::BinarySearch<Double_t>(N, T, t);
    if (i0 < 0 || i0 > N - 1)
      throw(_eh + "index out of range.");
    if (i0 == N - 1) // Happens as storage is in floats.
      i0 = N - 2;
  }

  const Double_t  f0 = (T[i0+1] - t) / (T[i0+1] - T[i0]);
  const Double_t  f1 = 1.0 - f0;

  const Double_t* P0 = S->GetY(i0);
  const Double_t* P1 = S->GetY(i0+1);

  Int_t ri = 0;
  Stepper<CosmicBall> stepper(*mBalls);
  while (stepper.step())
  {
    stepper->SetPos(f0*P0[ri]     + f1*P1[ri],
                    f0*P0[ri + 1] + f1*P1[ri + 1],
                    f0*P0[ri + 2] + f1*P1[ri + 2]);
    ri += 3;
    stepper->SetV  (f0*P0[ri]     + f1*P1[ri],
                    f0*P0[ri + 1] + f1*P1[ri + 1],
                    f0*P0[ri + 2] + f1*P1[ri + 2]);
    ri += 3;
  }

  mTime = t;
  Stamp(FID());
}

/**************************************************************************/

void SolarSystem::MakeStar()
{
  static const string _eh("SolarSystem::MakeStar ");

  assert_balls(_eh);

  CosmicBall* cb = new CosmicBall("Stella");
  cb->SetColor(1, 1, 0);
  cb->SetRadius(100);
  cb->SetLOD(20);

  // !!!! missing mass (add to mStarMass).
  Double_t mass = 1e9;
  mStarMass += mass;
  cb->SetM(mass);

  mQueen->CheckIn(cb);
  mBalls->Add(cb);
}

void SolarSystem::MakePlanetoid()
{
  static const string _eh("SolarSystem::MakePlanetoid ");

  assert_balls(_eh);

  using namespace TMath;

  Int_t idx = mBalls->Size();
  CosmicBall* cb = new CosmicBall(GForm("Planet %d", idx));

  Double_t r_planet    = gRandom->Uniform(mPlanetMinR, mPlanetMaxR);
  Double_t mass_planet = 4.0*Pi()*r_planet*r_planet*r_planet/3.0;
  cb->SetM(mass_planet);
  cb->SetRadius(r_planet);
  cb->SetLOD(16);
  cb->SetColor(0, 0.5, 0.7);

  Double_t r_orb     = gRandom->Uniform(mOrbitMinR, mOrbitMaxR);
  Double_t phi_orb   = gRandom->Uniform(0, TwoPi());
  Double_t theta_orb = DegToRad() * gRandom->Uniform(-mMaxTheta, mMaxTheta);
  Double_t cos_theta = Cos(theta_orb);

  TVector3 pos(r_orb * cos_theta * Cos(phi_orb),
	       r_orb * cos_theta * Sin(phi_orb),
	       r_orb * Sin(theta_orb));
  cb->SetPos(pos.x(), pos.y(), pos.z());
  pos.SetMag(1);

  TVector3 xplane(pos.Orthogonal());
  TVector3 yplane(pos.Cross(xplane));
  Double_t ecc     = gRandom->Uniform(0, mMaxEcc);
  Double_t phi_vel = gRandom->Uniform(0, TwoPi());
  Double_t vel_mag = Sqrt(mBallKappa*mStarMass/r_orb*(1 - ecc));

  xplane *= vel_mag * Cos(phi_vel);
  yplane *= vel_mag * Sin(phi_vel);

  cb->SetV(xplane.x() + yplane.x(), xplane.y() + yplane.y(), xplane.z() + yplane.z());

  mQueen->CheckIn(cb);
  mBalls->Add(cb);
}
