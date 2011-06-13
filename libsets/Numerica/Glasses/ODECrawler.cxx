// $Id$

#include "ODECrawler.h"
#include "ODECrawler.c7"

#include <TMath.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_odeiv.h>

//==============================================================================
// GSL -> OdeCrawlerMaster wrappers
//==============================================================================

namespace
{
  int ode_crawler_der_foo(double t, const double y[], double dydt[], void* ud)
  {
    ((ODECrawlerMaster*)ud)->ODEDerivatives(t, y, dydt);
    return GSL_SUCCESS;
  }

  int ode_crawler_jac_foo(double t, const double y[], double* dfdy, double dfdt[], void* ud)
  {
    ((ODECrawlerMaster*)ud)->ODEJacobian(t, y, dfdy, dfdt);
    return GSL_SUCCESS;
  }
}


//==============================================================================
// ODECrawlerMaster
//==============================================================================

//______________________________________________________________________________
//
// Abstract base class for glass that provides:
// a) ODE order
// b) initial conditions
// c) derivatives

ClassImp(ODECrawlerMaster);


//==============================================================================
// ODECrawler
//==============================================================================

//______________________________________________________________________________
//
// ODE integrator using adaptive step functions from GSL.
// X corresponds to time (scalar).
// Y corresponds to state vector at a given time.
// Final state can be obtained from mY.
// Works in double precision.
//
// Trajectory is stored in ODEstorage object with at least mStoreDx
// between consequtive points.
// Maximum of mStoreMax intermediate points are stored.
//   If mStoreMax = 0 no results are stored.
//   If mStoreMax < 0 all points are stored (default).
// Start and end points (at x1 and x2) are always stored, unless mStoreMax=0.
//
// Example glass Moonraker. Macro moonraker.C.
//________________________________________________________________________

ClassImp(ODECrawler);

void ODECrawler::_init()
{
  m_true_master = 0;
  m_crawling    = false;

  m_gsl_system  = new gsl_odeiv_system;
  m_gsl_system->function = ode_crawler_der_foo;
  m_gsl_system->jacobian = ode_crawler_jac_foo;

  m_gsl_evolve  = 0;
  m_gsl_control = 0;
  m_gsl_step    = 0;

  mStepOK = mStepChanged = mStored = 0;
  mMaxSteps  = 1000000; mStoreDx = 0.001; mStoreMax = -1;
  mStorage   = 0;
  mEpsAbs    = 0; mEpsRel = 1e-8; mFacVal = 1; mFacDer = 0;
  mH1 = 1e-2; mHmin = 1e-18; mH = 0; bStoreH = true;
  mStepFunc = SF_RKF45; mPrevStepFunc = (StepFunc_e) -1;
  mX1 = mX2 = 0;
}

ODECrawler::ODECrawler(const Text_t* n, const Text_t* t) :
  ZGlass(n,t)
{
  _init();
}

ODECrawler::~ODECrawler()
{
  delete mStorage;

  _gsl_free();
  delete m_gsl_system;
}

//==============================================================================

const gsl_odeiv_step_type* ODECrawler::s_get_step_func(ODECrawler::StepFunc_e sf)
{
  switch (sf)
  {
    case SF_RK2:    return gsl_odeiv_step_rk2;
    case SF_RK4:    return gsl_odeiv_step_rk4;
    case SF_RKF45:  return gsl_odeiv_step_rkf45;
    case SF_RKCK:   return gsl_odeiv_step_rkck;
    case SF_RK8PD:  return gsl_odeiv_step_rk8pd;
    case SF_RK2Imp: return gsl_odeiv_step_rk2imp;
    case SF_RK4Imp: return gsl_odeiv_step_rk4imp;
    case SF_BSImp:  return gsl_odeiv_step_bsimp;
    case SF_Gear1:  return gsl_odeiv_step_gear1;
    case SF_Gear2:  return gsl_odeiv_step_gear2;
    default:        return 0;
  }
}

void ODECrawler::_gsl_alloc()
{
  m_gsl_system->dimension = mN;
  m_gsl_system->params    = m_true_master;

  m_gsl_step    = gsl_odeiv_step_alloc(s_get_step_func(mStepFunc), mN);
  m_gsl_control = gsl_odeiv_control_standard_new(mEpsAbs, mEpsRel, mFacVal, mFacDer);
  m_gsl_evolve  = gsl_odeiv_evolve_alloc(mN);
}

void ODECrawler::_gsl_free()
{
  gsl_odeiv_evolve_free(m_gsl_evolve);
  gsl_odeiv_control_free(m_gsl_control);
  gsl_odeiv_step_free(m_gsl_step);
}

//==============================================================================

void ODECrawler::AdvanceXLimits(Double_t delta_x)
{
  // Advance x limits forward, so that X1 becomes X2 and new integration
  // interval is delta_x.
  // If delta_x is 0, length of the interval is preserved.

  if (delta_x == 0) delta_x = mX2 - mX1;
  mX1  = mX2;
  mX2 += delta_x;
}

void ODECrawler::SetStorage(ODEStorage* s)
{
  delete mStorage;
  mStorage = s;
}

void ODECrawler::DetachStorage()
{
  mStorage = 0;
}

ODEStorage* ODECrawler::SwapStorage(ODEStorage* s)
{
  ODEStorage* old = s;
  mStorage = s;
  return old;
}

//==============================================================================

void ODECrawler::init_integration(Bool_t call_ode_start)
{
  // Initialize storage arrays, clear integration state.
  // If call_ode_start is true, call ODEStart() in master to get
  // initial parameters.
  // m_true_master must be set.

  mN = m_true_master->ODEOrder();

  if (mY.GetNoElements() != mN || mStepFunc != mPrevStepFunc)
  {
    mY.ResizeTo(mN);
    _gsl_free();
    _gsl_alloc();
  }
  else
  {
    gsl_odeiv_control_init(m_gsl_control, mEpsAbs, mEpsRel, mFacVal, mFacDer);
    gsl_odeiv_evolve_reset(m_gsl_evolve);
  }

  if (mStoreMax != 0)
  {
    if (mStorage == 0)
    {
      mStorage = new ODEStorageD(mN);
    }
    else if (mStorage->Order() != mN)
    {
      mStorage->ResetOrder(mN);
    }
    else
    {
      mStorage->Reset();
    }
  }

  mStepOK = mStepChanged = mStored = 0;

  if (call_ode_start)
    m_true_master->ODEStart(mY.GetMatrixArray(), mX1, mX2);
}

//------------------------------------------------------------------------------

void ODECrawler::Integrate()
{
  // Integrate ODE with derivatives provided by mODEMaster from X1 to X2.

  static const Exc_t _eh("ODECrawler::Integrate ");

  Double_t x, h, h_last;
  Double_t xsav = 0;

  x = mX1;
  h = TMath::Sign((bStoreH && mH != 0) ? mH : mH1, mX2 - mX1);

  if (mStoreMax != 0)
  {
    mStorage->AddEntry(x, mY.GetMatrixArray());
    ++mStored;
    xsav = x;
  }

  Int_t n_step = 0;

  while (x != mX2)
  {
    if (++n_step > mMaxSteps)
    {
      throw _eh + GForm("Too many steps, MaxSteps=%d.", mMaxSteps);
    }

    h_last = h;

    int err = gsl_odeiv_evolve_apply(m_gsl_evolve, m_gsl_control, m_gsl_step,
				     m_gsl_system, &x, mX2, &h,
				     mY.GetMatrixArray());
    if (err != GSL_SUCCESS)
    {
      throw _eh + GForm("gsl error %d: %s", err, gsl_strerror(err));
    }

    if (mStoreMax < 0 ||
        (mStoreMax > 0 && mStored < mStoreMax - 1 &&
				    TMath::Abs(x - xsav) > mStoreDx))
    {
      mStorage->AddEntry(x, mY.GetMatrixArray());
      ++mStored;
      xsav = x;
    }

    if (h_last == h)
      ++mStepOK;
    else
      ++mStepChanged;

    if (TMath::Abs(h) <= mHmin)
    {
      throw _eh + GForm("Step size too small, h=%g.", h);
    }
  }

  if (mStoreMax != 0)
  {
    mStorage->AddEntry(x, mY.GetMatrixArray());
    ++mStored;
  }

  mH = bStoreH ? TMath::Abs(h) : 0;
}

/**************************************************************************/

void ODECrawler::Crawl(Bool_t call_ode_start)
{
  // Integrate with given master / parameters.
  // If called via a MIR it is executed in a broadcasted detached thread.
  //
  // The object is locked for the duration of the execution.

  static const Exc_t _eh("ODECrawler::Crawl ");

  {
    GLensReadHolder rdlock(this);

    if (m_crawling)
      throw _eh + "already crawling.";

    assert_odemaster(_eh);

    m_true_master = dynamic_cast<ODECrawlerMaster*>(*mODEMaster);
    if (!m_true_master)
      throw _eh + "master not an ODECrawlerMaster.";

    m_crawling = true;
  }

  Exc_t exc_p;
  try
  {
    init_integration(call_ode_start);
    Integrate();
  }
  catch (Exc_t& exc)
  {
    exc_p = exc;
  }

  {
    GLensReadHolder rdlock(this);

    m_crawling = false;
    Stamp(FID());
  }

  if (!exc_p.IsNull()) throw exc_p;
}

void ODECrawler::ChangeOrderInPlace(Int_t order)
{
  mN = order;
  if (mY.GetNoElements() != mN)
    mY.ResizeTo(mN);
}

Double_t* ODECrawler::RawYArray()
{
  return mY.GetMatrixArray();
}
