// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SolarSystem.h"
#include "SolarSystem.c7"

#include "CosmicBall.h"

#include <TVectorT.h>
#include <TRandom.h>
#include <TMath.h>

// SolarSystem

//__________________________________________________________________________
//
//

ClassImp(SolarSystem);

/**************************************************************************/

void SolarSystem::_init()
{
  mStarMass   = 0;
  mBallKappa  = 1e-6;

  mPlanetMinR = 20;
  mPlanetMaxR = 40;
  mOrbitMinR  = 150;
  mOrbitMaxR  = 1000;
  mMaxTheta   = 20;
  mMaxEcc     = 0.1;
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
    rfac = 1.0 / (rfac*TMath::Sqrt(rfac));
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

  x1 = 0; x2 = 1000;
}

/**************************************************************************/

void SolarSystem::MakeStar()
{
  static const string _eh("SolarSystem::MakeStar ");

  assert_balls(_eh);

  CosmicBall* cb = new CosmicBall("Stella");
  cb->SetColor(1, 1, 0);
  cb->SetRadius(100);

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
