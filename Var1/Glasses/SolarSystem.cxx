// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "SolarSystem.h"
#include "SolarSystem.c7"

#include "CosmicBall.h"
#include <Glasses/ZQueen.h>

#include <Gled/GThread.h>

#include <TVectorT.h>
#include <TRandom.h>
#include <TMath.h>
#include <TSystem.h>


//==============================================================================
// SolarSystem
//==============================================================================

//__________________________________________________________________________
//
// Simple simulation of a most unrealistic solar system.
//
//

ClassImp(SolarSystem);

/**************************************************************************/

void SolarSystem::_init()
{
  hWarnTimeOutOfRange = true;

  mTime    = 0;
  mTimeFac = 1;

  mBallHistorySize = 0;

  mStarMass   = 0;
  mBallKappa  = 1e-6;

  mPlanetMinR = 20;
  mPlanetMaxR = 40;
  mOrbitMinR  = 200;
  mOrbitMaxR  = 1000;
  mMaxTheta   = 20;
  mMaxEcc     = 0.2;

  mTimePerChunk  = 1000;
  mKeepPast      = 2000;
  mCalcFuture    = 5000;

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
  const Int_t max_i = ODEOrder();

  for (Int_t i = 0; i < max_i; i += 6)
  {
    d(i)   = y(i+3);
    d(i+1) = y(i+4);
    d(i+2) = y(i+5);

    d(i+3) = 0;
    d(i+4) = 0;
    d(i+5) = 0;
  }

  TVector3 delta;

  // Between cosmic balls: m * kappa / r^2
  // Sum contributions from the tail of mBalls as the biggest
  // contribution will come from Stella, which is usually at position 0.

  Int_t iMax = mBalls->Size() - 1;
  for (Int_t i = iMax; i >= 0; --i)
  {
    const Int_t    a  = 6*i;
    CosmicBall*    Bi = (CosmicBall*) (**mBalls)[i];

    for (Int_t j = iMax; j > i; --j)
    {
      const Int_t    b  = 6*j;
      CosmicBall*    Bj = (CosmicBall*) (**mBalls)[j];

      delta.SetXYZ(y(b) - y(a), y(b+1) - y(a+1), y(b+2) - y(a+2));

      const Double_t rsqr  = delta.Mag2();
      const Double_t R     = Bi->mRadius + Bi->mRadius;
      const Double_t Rsqr  = R * R;
      Double_t rftot;
      if (rsqr > Rsqr)
      {
        rftot = mBallKappa / (rsqr*TMath::Sqrt(rsqr));
      }
      else
      {
        rftot = mBallKappa * TMath::Sqrt(rsqr) / (Rsqr*Rsqr);
      }

      const Double_t fi = Bj->mM * rftot;
      d(a+3) += fi * delta.x();
      d(a+4) += fi * delta.y();
      d(a+5) += fi * delta.z();
      const Double_t fj = Bi->mM * rftot;
      d(b+3) -= fj * delta.x();
      d(b+4) -= fj * delta.y();
      d(b+5) -= fj * delta.z();
    }
  }

  // Brutal fix - keep Stella at the origin.
  // Should calculate SS speed and put this into its gallactic node.
  // Assumes a single star at index 0.
  d(0) = d(1) = d(2) = d(3) = d(4) = d(5) = 0;
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

  Double_t tdt = mTimeFac*dt;

  if (tdt == 0)
    return;

  Double_t new_time = mTime + tdt;

  // Here we could check how much time has elapsed since last position store
  // and decide whether to store this time.
  // But need store dt, last_store_t members.
  mTime = set_time(new_time, true);
  Stamp(FID());
}

/**************************************************************************/

SolarSystem::Storage* SolarSystem::find_storage_from_time(Double_t t)
{
  Storage* s = mCurrentStorage->second;
  if (t >= s->GetMinXStored() && t <= s->GetMaxXStored())
    return s;

  {
    GMutexHolder lock_storage(mStorageCond);

    mCurrentStorage = mStorageMap.lower_bound(t);
    if (mCurrentStorage != mStorageMap.begin())
    {
      --mCurrentStorage;
      mStorageCond.Signal();
    }
    else
    {
      // We are at the beginning.
      // As we don't have backward-propagation, it makes no
      // sense to signal storage condition.
    }
  }

  return mCurrentStorage->second;
}

Double_t SolarSystem::set_time(Double_t t, Bool_t from_timetick)
{
  // Place balls into position for time t.
  // Linear extrapolation between two stored points is made.
  //
  // When from_timetick is true the history of the balls is updated.
  // Otherwise it is reset to 0.

  static const Exc_t _eh("SolarSystem::SetTime ");

  Storage        *S = find_storage_from_time(t);
  const Double_t *T = S->GetXArr();
  const Int_t     N = S->Size();

  Int_t i0;
  if (t <= T[0])
  {
    if (t < T[0] && hWarnTimeOutOfRange) {
      ISwarn(_eh + GForm("given time %f below minimum %f. Using minimum.", t, T[0]));
      hWarnTimeOutOfRange = false;
    }
    i0 = 0;
    t  = T[0];
  }
  else if (t >= T[N-1])
  {
    if (t > T[N-1] && hWarnTimeOutOfRange) {
      ISwarn(_eh + GForm("given time %f above maximum %f. Using maximum", t, T[N-1]));
      hWarnTimeOutOfRange = false;
    }
    i0 = N - 2;
    t  = T[N-1];
  }
  else
  {
    hWarnTimeOutOfRange = true;

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

  Int_t  ri        = 0;
  Bool_t store_pos = from_timetick && mBallHistorySize > 0;
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

    if (store_pos)
    {
      stepper->StorePos();
    }

    ri += 3;
  }

  return t;
}

void SolarSystem::SetTime(Double_t t)
{
  mTime = set_time(t, false);
  Stepper<CosmicBall> stepper(*mBalls);
  while (stepper.step())
  {
    stepper->ClearHistory();
  }
  Stamp(FID());
}

void SolarSystem::SetBallHistorySize(Int_t history_size)
{
  if (history_size < 0)    history_size = 0;
  if (history_size > 4096) history_size = 4096;
  mBallHistorySize = history_size;

  Stepper<CosmicBall> stepper(*mBalls);
  while (stepper.step())
    stepper->ResizeHistory(mBallHistorySize);

  Stamp(FID());
}

/**************************************************************************/

void* SolarSystem::tl_IntegratorThread(SolarSystem* ss)
{
  ss->IntegratorThread();
  return 0;
}

void SolarSystem::IntegratorThread()
{
  static const Exc_t _eh("SolarSystem::IntegratorThread ");

  while (true)
  {
    Storage* old_storage = (Storage*) mODECrawler->GetStorage();
    Storage* new_storage = new Storage(*old_storage);

    mODECrawler->SwapStorage(new_storage);
    mODECrawler->AdvanceXLimits(mTimePerChunk);

    mODECrawler->Crawl(false);

    {
      GMutexHolder lock_storage(mStorageCond);

      mStorageMap.insert(make_pair(new_storage->GetMinXStored(),
                                       new_storage));

      bool calc_needed = false;
      while (!calc_needed)
      {
        Double_t t_min = mTime - mKeepPast;
        while (mStorageMap.begin()->second->GetMaxXStored() < t_min)
        {
          printf("%sRemoving storage [%f, %f], t=%f, t_min=%f\n",
                 _eh.Data(),
                 mStorageMap.begin()->second->GetMinXStored(),
                 mStorageMap.begin()->second->GetMaxXStored(),
                 mTime, t_min);
          mStorageMap.erase(mStorageMap.begin());
        }

        Double_t t_max = mTime + mCalcFuture;
        if (mStorageMap.rbegin()->second->GetMinXStored() < t_max)
        {
          calc_needed = true;
          printf("%sRequesting calculation, max_available=%f t_max=%f...\n",
                 _eh.Data(),
                 mStorageMap.rbegin()->second->GetMaxXStored(),
                 t_max);
        }
        else
        {
          printf("%sNothing to be done ... waiting.\n", _eh.Data());
          mStorageCond.Wait();
          printf("%sSignal received ... let's see ...\n", _eh.Data());
        }
      }
    }
  }
}

void SolarSystem::StartIntegratorThread()
{
  // Start the ODE integrator thread.
  // Throws an exception if the thread is already running.
  //
  // This expects that the CosmicBalls are registered into this object.
  // All existing storage is dropped, the time starts at zero.
  // ODECrawler is initialized and the first chunk is calculated in the
  // calling thread.
  // After that, a new thread is spawned with nice value of 10.

  static const Exc_t _eh("SolarSystem::StartIntegratorThread ");

  assert_odecrawler(_eh);

  if (mIntegratorThread)
    throw (_eh + "already running.");

  mStorageMap.clear();
  mCurrentStorage = mStorageMap.end();

  mODECrawler->SetX1(0);
  mODECrawler->SetX2(mTimePerChunk);

  Storage* s = new Storage(ODEOrder());
  mODECrawler->SetStorage(s);
  mODECrawler->Crawl();

  mCurrentStorage = mStorageMap.insert(make_pair(0, s)).first;

  mIntegratorThread = new GThread("SolarSystem-Integrator",
                                  (GThread_foo) (tl_IntegratorThread), this,
                                  false);
  mIntegratorThread->SetNice(10);
  mIntegratorThread->Spawn();
}

void SolarSystem::StopIntegratorThread()
{
  // not finished

  static const Exc_t _eh("SolarSystem::StopIntegratorThread ");

  if (mIntegratorThread == 0)
    throw (_eh + "not running.");

  mIntegratorThread->Cancel();
  mIntegratorThread->Join();
  mIntegratorThread = 0;
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

  Double_t ecc     = gRandom->Uniform(0, mMaxEcc);
  Double_t vel_mag = Sqrt(mBallKappa*mStarMass/r_orb*(1 - ecc));

  const Double_t vfac = vel_mag / pos.Mag();
  cb->SetV(-pos.y()*vfac, pos.x()*vfac, 0);

  // Older calculation with random phi velocity.
  //
  // Double_t phi_vel = gRandom->Uniform(0, TwoPi());
  // TVector3 xplane(pos.Orthogonal());
  // TVector3 yplane(pos.Cross(xplane));
  // xplane *= vel_mag * Cos(phi_vel);
  // yplane *= vel_mag * Sin(phi_vel);
  // cb->SetV(xplane.x() + yplane.x(), xplane.y() + yplane.y(), xplane.z() + yplane.z());

  mQueen->CheckIn(cb);
  mBalls->Add(cb);
}