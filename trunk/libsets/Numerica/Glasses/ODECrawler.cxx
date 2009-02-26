// $Id$

#include "ODECrawler.h"
#include "ODECrawler.c7"

#include <TMath.h>


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
// ODE integrator built upon adaptive step 4th order runge-kutta.
// Based on odeint from numerical recipes.
// X corresponds to time (scalar).
// Y corresponds to state vector at a given time.
// Final state can be obtained from mY.
// Works in double precision.
//
// No good way for permanent storage or broadcasting of results yet.
//
// Trajectory is stored in ODEstorage object with at least mStoreDx
// between consequtive points.
// Maximum of mStoreMax intermediate points are stored.
// If mStoreMax = 0 no results are stored.
// If mStoreMax < 0 all points are stored (default).
//
// Example glass Moonraker. Macro moonraker.C.
//________________________________________________________________________

ClassImp(ODECrawler);

void ODECrawler::_init()
{
  hTINY = 1e-30; hSAFETY = 0.9; hPGROW = -0.2; hPSHRNK = -0.25; hERRCON = 1.89e-4;
  hTrueMaster = 0;
  hCrawling   = false;

  mGuessesOK = mGuessesBad = mStored = 0;
  mMaxSteps  = 1000000; mStoreDx = 0.001; mStoreMax = -1;
  mStorage   = 0;
  mAcc = 1e-8; mH1 = 1e-2; mHmin = 1e-18;
  mX1 = 0; mX2 = 0;
}

ODECrawler::~ODECrawler()
{
  delete mStorage;
}

/**************************************************************************/

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

/**************************************************************************/

void ODECrawler::init_integration(Bool_t call_ode_start)
{
  // Initialize storage arrays, clear integration state.
  // If call_ode_start is true, call ODEStart() in master to get
  // initial parameters.
  // hTrueMaster must be set.

  mN = hTrueMaster->ODEOrder();
  if (mY.GetNoElements() != mN)
    mY.ResizeTo(mN);

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

  mGuessesOK = mGuessesBad = mStored = 0;

  if (call_ode_start)
    hTrueMaster->ODEStart(mY, mX1, mX2);
}

Int_t
ODECrawler::Rkqs(TVectorD& y, TVectorD& dydx, Double_t& x, Double_t htry,
		 TVectorD& yscal, Double_t& hdid, Double_t& hnext)
{
  static const Exc_t _eh("ODECrawler::Rkqs ");

  Double_t errmax,htemp,xnew;

  TVectorD yerr(mN), ytemp(mN);
  Double_t h=htry;
  while (true)
  {
    Rkck(y, dydx, x, h, ytemp, yerr);
    errmax = 0.0;
    for (Int_t i=0; i<mN; ++i)
      errmax = TMath::Max(errmax,TMath::Abs(yerr(i)/yscal(i)));
    errmax /= mAcc;
    if (errmax <= 1.0) break;
    htemp = hSAFETY*h*TMath::Power(errmax,hPSHRNK);
    h = (h >= 0.0 ? TMath::Max(htemp,0.1*h) : TMath::Min(htemp,0.1*h));
    xnew = x + h;
    if (xnew == x)
    {
      throw(_eh + GForm("stepsize underflow, errmax=%f.", errmax));
    }
  }
  if (errmax > hERRCON)
    hnext = hSAFETY*h*TMath::Power(errmax,hPGROW);
  else
    hnext = 5.0*h;
  x += (hdid=h);
  y = ytemp;
  return 0;
}

/**************************************************************************/

void ODECrawler::Rkck(TVectorD& y, TVectorD& dydx, Double_t x, Double_t h,
                      TVectorD& yout, TVectorD& yerr)
{
  static const Double_t a2=0.2, a3=0.3, a4=0.6, a5=1.0, a6=0.875,
    b21=0.2, b31=3.0/40.0, b32=9.0/40.0, b41=0.3, b42=-0.9, b43=1.2,
    b51=-11.0/54.0, b52=2.5,b53=-70.0/27.0, b54=35.0/27.0,
    b61=1631.0/55296.0, b62=175.0/512.0, b63=575.0/13824.0,
    b64=44275.0/110592.0, b65=253.0/4096.0, c1=37.0/378.0,
    c3=250.0/621.0, c4=125.0/594.0, c6=512.0/1771.0, dc5=-277.00/14336.0;
  static const Double_t dc1=c1-2825.0/27648.0, dc3=c3-18575.0/48384.0,
    dc4=c4-13525.0/55296.0, dc6=c6-0.25;

  TVectorD ak2(mN), ak3(mN), ak4(mN), ak5(mN), ak6(mN), ytemp(mN);

  for (Int_t i=0; i<mN; ++i)
    ytemp(i) = y(i) + b21*h*dydx(i);
  hTrueMaster->ODEDerivatives(x+a2*h,ytemp,ak2);
  for (Int_t i=0; i<mN; ++i)
    ytemp(i)=y(i)+h*(b31*dydx(i)+b32*ak2(i));
  hTrueMaster->ODEDerivatives(x+a3*h,ytemp,ak3);
  for (Int_t i=0; i<mN; ++i)
    ytemp(i)=y(i)+h*(b41*dydx(i)+b42*ak2(i)+b43*ak3(i));
  hTrueMaster->ODEDerivatives(x+a4*h,ytemp,ak4);
  for (Int_t i=0; i<mN; ++i)
    ytemp(i)=y(i)+h*(b51*dydx(i)+b52*ak2(i)+b53*ak3(i)+b54*ak4(i));
  hTrueMaster->ODEDerivatives(x+a5*h,ytemp,ak5);
  for (Int_t i=0; i<mN; ++i)
    ytemp(i)=y(i)+h*(b61*dydx(i)+b62*ak2(i)+b63*ak3(i)+b64*ak4(i)+b65*ak5(i));
  hTrueMaster->ODEDerivatives(x+a6*h,ytemp,ak6);
  for (Int_t i=0; i<mN; ++i)
    yout(i)=y(i)+h*(c1*dydx(i)+c3*ak3(i)+c4*ak4(i)+c6*ak6(i));
  for (Int_t i=0; i<mN; ++i)
    yerr(i)=h*(dc1*dydx(i)+dc3*ak3(i)+dc4*ak4(i)+dc5*ak5(i)+dc6*ak6(i));
}

/**************************************************************************/

void ODECrawler::Integrate()
{
  // Integrate ODE with derivatives provided by mODEMaster from X1 to X2.

  static const Exc_t _eh("ODECrawler::Integrate ");

  Double_t x, h, hdid, hnext;
  Double_t xsav = 0;

  x = mX1;
  h = TMath::Sign(mH1, mX2-mX1);
  TVectorD y(mY);
  TVectorD yscal(mN), dydx(mN);
  if (mStoreMax != 0) xsav = x - 2*mStoreDx;
  for (Int_t nstp=0; nstp<mMaxSteps; ++nstp)
  {
    hTrueMaster->ODEDerivatives(x, y, dydx);
    for (Int_t i=0; i<mN; ++i)
    {
      yscal(i) = TMath::Abs(y(i)) + TMath::Abs(dydx(i)*h) + hTINY;
    }

    if (mStoreMax < 0 ||
        (mStoreMax > 0 && mStored < mStoreMax - 1 &&
         TMath::Abs(x - xsav) > TMath::Abs(mStoreDx)))
    {
      mStorage->AddEntry(x, y.GetMatrixArray());
      ++mStored;
      xsav = x;
    }

    if ((x + h - mX2)*(x + h - mX1) > 0.0) h = mX2 - x;

    if (Rkqs(y, dydx, x, h, yscal, hdid, hnext)) return;

    if (hdid == h)
      ++mGuessesOK;
    else
      ++mGuessesBad;

    if ((x - mX2)*(mX2 - mX1) >= 0.0)
    {
      mY = y;
      if (mStoreMax != 0)
      {
        mStorage->AddEntry(x, y.GetMatrixArray());
	++mStored;
      }
      return;
    }
    if (TMath::Abs(hnext) <= mHmin)
    {
      throw (_eh + GForm("Step size too small, hnext=%g.", hnext));
    }
    h = hnext;
  }
  throw (_eh + GForm("Too many steps, MaxSteps=%d.", mMaxSteps));
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

    if (hCrawling)
      throw(_eh + "already crawling.");

    assert_odemaster(_eh);

    hTrueMaster = dynamic_cast<ODECrawlerMaster*>(*mODEMaster);
    if (!hTrueMaster)
      throw(_eh + "master not an ODECrawlerMaster.");

    hCrawling = true;
  }

  init_integration(call_ode_start);
  Integrate();

  {
    GLensReadHolder rdlock(this);

    hCrawling = false;
    Stamp(FID());
  }
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
