// $Id$

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
// The integration of the system can be performed in two modes.
//
// 1) ChunkedStorage
//
//    The integrator runs in a separate thread and stores the results
//    for given past/future in the storage map (see members mKeepPast
//    and mCalcFuture). The time can be selected from outside by
//    calling SetTime().
//
// 2) DirectStep
//
//    The calculation is performed on each TimeTick and results are
//    put directly back into the CosmicBalls. There is no way to set
//    the time otherwise.
//
// Do NOT change the mode after the processing has started ... or you
// will be sorry. Anyway ... they require a completely different setup
// for an efficient update-render loop.

ClassImp(SolarSystem);

/**************************************************************************/

void SolarSystem::_init()
{
  hWarnTimeOutOfRange = true;
  hStepIntegratorDt   = 0;

  mCrawlMode = CM_ChunkedStorage;

  mTime    = 0;
  mTimeFac = 1;

  mBallHistorySize = 0;

  mStarMass   = 0;
  mBallKappa  = 1e-6;

  mPlanetRnd.SetSeed(0);
  mPlanetMinR = 10;
  mPlanetMaxR = 50;
  mOrbitMinR  = 200;
  mOrbitMaxR  = 1000;
  mMaxTheta   = 20;
  mMaxEcc     = 0.2;
  mPlanetColor.rgba(0, 0.7, 1);

  mTimePerChunk  = 1000;
  mKeepPast      = 2000;
  mCalcFuture    = 5000;

  mIntegratorThread = 0;

  bDesiredRHack   = false;
  mDesiredRHackT0 = 5; // This is in real time, time-fac is ignored!
}

SolarSystem::SolarSystem(const Text_t* n, const Text_t* t) :
  ZNode(n, t)
{
  _init();
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
    l->SetMIRActive(false);
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
  static const Exc_t _eh("SolarSystem::ODEOrder ");

  assert_balls(_eh);
  return 6 * mBalls->Size();
}

void SolarSystem::ODEStart(Double_t y[], Double_t& x1, Double_t& x2)
{
  Int_t i = 0;
  Stepper<CosmicBall> stepper(*mBalls);
  while (stepper.step())
  {
    const Double_t* P = stepper->RefTrans().ArrT();
    const TVector3& V = stepper->RefV();

    y[i++] = P[0];  y[i++] = P[1];  y[i++] = P[2];
    y[i++] = V.x(); y[i++] = V.y(); y[i++] = V.z();
  }

  assert(i == (Int_t) ODEOrder());
}

void SolarSystem::ODEDerivatives(Double_t x, const Double_t y[], Double_t d[])
{
  const Int_t max_i = ODEOrder();

  for (Int_t i = 0; i < max_i; i += 6)
  {
    d[i]   = y[i+3];
    d[i+1] = y[i+4];
    d[i+2] = y[i+5];

    d[i+3] = 0;
    d[i+4] = 0;
    d[i+5] = 0;
  }

  TVector3 delta;

  // Between cosmic balls: m * kappa / r^2
  // Sum contributions from the tail of mBalls as the biggest
  // contribution will come from Stella, which is usually at position 0.

  const Int_t iMax = mBalls->Size() - 1;
  for (Int_t i = iMax; i >= 0; --i)
  {
    CosmicBall *Bi = (CosmicBall*) (**mBalls)[i];
    if (Bi == 0)
      continue;

    const Int_t a = 6*i;

    for (Int_t j = iMax; j > i; --j)
    {
      CosmicBall *Bj = (CosmicBall*) (**mBalls)[j];
      if (Bj == 0)
	continue;

      const Int_t b = 6*j;

      delta.SetXYZ(y[b] - y[a], y[b+1] - y[a+1], y[b+2] - y[a+2]);

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
      d[a+3] += fi * delta.x();
      d[a+4] += fi * delta.y();
      d[a+5] += fi * delta.z();
      const Double_t fj = Bi->mM * rftot;
      d[b+3] -= fj * delta.x();
      d[b+4] -= fj * delta.y();
      d[b+5] -= fj * delta.z();
    }
  }

  // Brutal fix - keep Stella at the origin.
  // Should calculate SS speed and put this into its gallactic node.
  // Assumes a single star at index 0.
  d[0] = d[1] = d[2] = d[3] = d[4] = d[5] = 0;
}

/**************************************************************************/

void SolarSystem::TimeTick(Double_t t, Double_t dt)
{
  GLensWriteHolder wrlck(this);

  Double_t tdt = mTimeFac*dt;

  if (tdt == 0)
    return;

  switch (mCrawlMode)
  {
    case CM_ChunkedStorage:
    {
      Double_t new_time = mTime + tdt;

      // Here we could check how much time has elapsed since last position store
      // and decide whether to store this time.
      // But need store dt, last_store_t members.
      mTime = set_time(new_time, true);
      Stamp(FID());
      break;
    }
    case CM_DirectStep:
    {
      {
	GMutexHolder lock_storage(mStorageCond);

	if (bDesiredRHack)
	{
	  hack_desired_r(dt);
	}

	{
	  const Double_t *Y = mODECrawler->RefY().GetMatrixArray();
	  Bool_t store_pos  = mBallHistorySize > 0;
	  Stepper<CosmicBall> stepper(*mBalls, true);
	  while (stepper.step())
	  {
	    if (stepper.get_lens() != 0)
	    {
	      stepper->SetPos(Y[0], Y[1], Y[2]);
	      stepper->SetV  (Y[3], Y[4], Y[5]);

	      if (store_pos)
	      {
		stepper->StorePos();
	      }
	    }
	    Y += 6;
	  }
	}

	{
	  GMutexHolder    lock_switcher(mBallSwitchMutex);
	  GLensReadHolder lock_balls   (*mBalls);

	  while ( ! mBallsToRemove.empty())
	  {
	    mBalls->RemoveAll(mBallsToRemove.front());
	    mBallsToRemove.pop_front();
	  }
	  if ( ! mBallsToAdd.empty())
	  {
	    Int_t miss = mBallsToAdd.size() - mBalls->CountEmptyIds();
	    if (miss > 0)
	    {
	      const Int_t new_size = mBalls->Size() + miss;
	      mBalls->Resize(new_size);
	      mODECrawler->ChangeOrderInPlace(6*new_size);
	    }

	    Double_t* arr = mODECrawler->RawYArray();
	    Int_t     idx = 0;
	    while ( ! mBallsToAdd.empty())
	    {
	      idx = mBalls->FindFirstEmptyId(idx);
	      assert(idx != -1);
	      CosmicBall* cb = mBallsToAdd.front();
              cb->ResizeHistory(mBallHistorySize);
	      mBalls->SetElementById(cb, idx);

	      const Double_t* T = cb->RefTrans().ArrT();
	      const TVector3& V = cb->RefV();
	      const Int_t     i = 6*idx;
	      arr[i]   = T[0];  arr[i+1] = T[1];  arr[i+2] = T[2];
	      arr[i+3] = V.x(); arr[i+4] = V.y(); arr[i+5] = V.z();

	      mBallsToAdd.pop_front();
	    }
	  }
	}

	mTime += hStepIntegratorDt;
	hStepIntegratorDt = tdt;

	mStorageCond.Signal();
      }

      Stamp(FID());
      break;
    }
  }
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
      throw _eh + "index out of range.";
    if (i0 == N - 1) // Happens as storage is in floats.
      i0 = N - 2;
  }

  const Double_t  f0 = (T[i0+1] - t) / (T[i0+1] - T[i0]);
  const Double_t  f1 = 1.0 - f0;

  const Double_t* P0 = S->GetY(i0);
  const Double_t* P1 = S->GetY(i0+1);

  // Assumes contiguous population of balls!
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
  static const Exc_t _eh("SolarSystem::SetTime ");

  if (mCrawlMode == CM_DirectStep)
    throw _eh + "Not available in direct-step mode.";

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
  switch (ss->mCrawlMode)
  {
    case CM_ChunkedStorage:
      ss->ChunkIntegratorThread();
      break;
    case CM_DirectStep:
      ss->StepIntegratorThread();
      break;
  }
  return 0;
}

void SolarSystem::ChunkIntegratorThread()
{
  static const Exc_t _eh("SolarSystem::ChunkIntegratorThread ");

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
	  delete mStorageMap.begin()->second;
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

void SolarSystem::StepIntegratorThread()
{
  static const Exc_t _eh("SolarSystem::StepIntegratorThread ");

  GMutexHolder lock_storage(mStorageCond);

  while (true)
  {
    mStorageCond.Wait();

    mODECrawler->AdvanceXLimits(hStepIntegratorDt);
    mODECrawler->Crawl(false);
  }
}

//==============================================================================

void SolarSystem::StartChunkIntegratorThread()
{
  // Start the ODE integrator thread.
  // Throws an exception if the thread is already running.
  //
  // This expects that the CosmicBalls are registered into this object.
  // All existing storage is dropped, the time starts at zero.
  // ODECrawler is initialized and the first chunk is calculated in the
  // calling thread.
  // After that, a new thread is spawned with nice value of 10.

  static const Exc_t _eh("SolarSystem::StartChunkIntegratorThread ");

  assert_odecrawler(_eh);

  if (mCrawlMode != CM_ChunkedStorage)
    throw _eh + "Not in ChunkedStorage mode.";

  if (mIntegratorThread)
    throw _eh + "Integrator thread already running.";

  mStorageMap.clear();
  mCurrentStorage = mStorageMap.end();

  mODECrawler->SetX1(mTime);
  mODECrawler->SetX2(mTime + mTimePerChunk);

  Storage* s = new Storage(ODEOrder());
  mODECrawler->SetStorage(s);
  mODECrawler->Crawl();

  mCurrentStorage = mStorageMap.insert(make_pair(0, s)).first;

  mIntegratorThread = new GThread("SolarSystem-ChunkIntegrator",
                                  (GThread_foo) (tl_IntegratorThread), this,
                                  false);
  mIntegratorThread->SetNice(10);
  mIntegratorThread->Spawn();
}

void SolarSystem::StartStepIntegratorThread()
{
  static const Exc_t _eh("SolarSystem::StartStepIntegratorThread ");

  assert_odecrawler(_eh);

  if (mCrawlMode == CM_DirectStep)
    throw _eh + "Already in direct-step mode.";

  if (mIntegratorThread)
    throw _eh + "Integrator thread already running.";

  mCrawlMode = CM_DirectStep;
  {
    GLensReadHolder rdlock(*mODECrawler);

    mODECrawler->SetX1(mTime);
    mODECrawler->SetX2(mTime);
    mODECrawler->SetStoreMax(0);
  }
  // Make a phony step ... with initialization from the balls.
  mODECrawler->Crawl();

  hStepIntegratorDt = 0;

  mIntegratorThread = new GThread("SolarSystem-StepIntegrator",
                                  (GThread_foo) (tl_IntegratorThread), this,
                                  false);
  mIntegratorThread->SetNice(10);
  mIntegratorThread->Spawn();

  Stamp(FID());
}

void SolarSystem::StopIntegratorThread()
{
  // not finished

  static const Exc_t _eh("SolarSystem::StopIntegratorThread ");

  if (mIntegratorThread == 0)
    throw _eh + "not running.";

  mIntegratorThread->Cancel();
  mIntegratorThread->Join();
  mIntegratorThread = 0;
}

/**************************************************************************/

CosmicBall* SolarSystem::RandomPlanetoid(const TString& name)
{
  // Generate a random planetoid with given name.

  using namespace TMath;

  CosmicBall* cb = new CosmicBall(name);

  Double_t r_planet    = mPlanetRnd.Uniform(mPlanetMinR, mPlanetMaxR);
  Double_t mass_planet = 4.0*Pi()*r_planet*r_planet*r_planet/3.0;

  cb->SetM(mass_planet);
  cb->SetRadius(r_planet);
  cb->SetLOD(16);
  cb->SetColorByRef(mPlanetColor);

  Double_t r_orb     = mPlanetRnd.Uniform(mOrbitMinR, mOrbitMaxR);
  Double_t phi_orb   = mPlanetRnd.Uniform(0, TwoPi());
  Double_t theta_orb = DegToRad() * mPlanetRnd.Uniform(-mMaxTheta, mMaxTheta);
  Double_t cos_theta = Cos(theta_orb);

  TVector3 pos(r_orb * cos_theta * Cos(phi_orb),
	       r_orb * cos_theta * Sin(phi_orb),
	       r_orb * Sin(theta_orb));
  cb->SetPos(pos.x(), pos.y(), pos.z());

  Double_t ecc     = mPlanetRnd.Uniform(0, mMaxEcc);
  Double_t vel_mag = Sqrt(mBallKappa*mStarMass/r_orb*(1 - ecc));

  const Double_t vfac = vel_mag / pos.Mag();
  cb->SetV(-pos.y()*vfac, pos.x()*vfac, 0);

  // Older calculation with random phi velocity.
  //
  // Double_t phi_vel = mPlanetRnd.Uniform(0, TwoPi());
  // TVector3 xplane(pos.Orthogonal());
  // TVector3 yplane(pos.Cross(xplane));
  // xplane *= vel_mag * Cos(phi_vel);
  // yplane *= vel_mag * Sin(phi_vel);
  // cb->SetV(xplane.x() + yplane.x(), xplane.y() + yplane.y(), xplane.z() + yplane.z());

  return cb;
}

void SolarSystem::MakeStar()
{
  // Not cluster safe.

  static const Exc_t _eh("SolarSystem::MakeStar ");

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
  // Not cluster safe.

  static const Exc_t _eh("SolarSystem::MakePlanetoid ");

  assert_balls(_eh);

  Int_t idx = mBalls->Size();
  CosmicBall *cb = RandomPlanetoid(GForm("Planet %d", idx));
  cb->ResizeHistory(mBallHistorySize);

  mQueen->CheckIn(cb);
  mBalls->Add(cb);
}

void SolarSystem::AddPlanetoid(CosmicBall* cb)
{
  // Add planetoid cb.
  // If integrator thread is already running, this is only possible
  // in the direct-step mode.
  // The ball should be already checked-in.

  static const Exc_t _eh("SolarSystem::AddPlanetoid ");

  if (mIntegratorThread && mCrawlMode != CM_DirectStep)
    throw _eh + "Balls can be added during thread operation only in direct-step mode.";

  GMutexHolder ball_lock(mBallSwitchMutex);

  mBallsToAdd.push_back(cb);
}

void SolarSystem::RemovePlanetoid(CosmicBall* cb)
{
  // Remove planetoid cb.
  // If integrator thread is already running, this is only possible
  // in the direct-step mode.
  // If the ball is not referenced from anywhere else it is at risk
  // of being auto-destructed.

  static const Exc_t _eh("SolarSystem::RemovePlanetoid ");

  if (mIntegratorThread && mCrawlMode != CM_DirectStep)
    throw _eh + "Balls can be removed during thread operation only in direct-step mode.";

  GMutexHolder ball_lock(mBallSwitchMutex);

  mBallsToRemove.push_back(cb);
}

//==============================================================================

void SolarSystem::hack_desired_r(Double_t dt)
{
  const Double_t eps = 0.01;
  const Double_t tf  = dt / mDesiredRHackT0;

  Double_t *Y  = mODECrawler->RawYArray();

  Stepper<CosmicBall> stepper(*mBalls, true);
  while (stepper.step())
  {
    if (stepper.get_lens() != 0 && stepper->GetDesiredR() != 0)
    {
      const Double_t r_des = stepper->GetDesiredR();
      const Double_t r_mag = TMath::Sqrt(Y[0]*Y[0] + Y[1]*Y[1] + Y[2]*Y[2]);

      Double_t d = (r_des - r_mag) / r_des;
      if (d < -eps || d > eps)
      {
	d *= tf;
	Y[0] += d*Y[0]; Y[1] += d*Y[1]; Y[2] += d*Y[2];

	d = 1.0 / TMath::Sqrt(1 + d);
	Y[3] *= d; Y[4] *= d; Y[5] *= d;
      }
      else
      {
	stepper->SetDesiredR(0);
      }
    }
    Y += 6;
  }

}
