// $Header$

//________________________________________________________________________
// ODECrawler
//
// ODE integrator built upon adaptive step 4th order runge-kutta.
// Based on odeint from numerical recipes.
// X corresponds to time (scalar).
// Y corresponds to state vector at a given time.
// Works in double precision.
//
// No good way for permanent storage or broadcasting of results yet.
// Trajectory is stored in mXStored, mYstored;
// Final state can be obtained from mY.
//
// Example glass Moonraker. Macro moonraker.C.
//________________________________________________________________________

#include "ODECrawler.h"
#include <TMath.h>

ClassImp(ODECrawlerMaster)
ClassImp(ODECrawler)

void ODECrawler::_init()
{
  hTINY = 1e-30; hSAFETY = 0.9; hPGROW = -0.2; hPSHRNK = -0.25; hERRCON = 1.89e-4;
  mGuessesOK = mGuessesBad = mStored = 0;
  mMaxSteps = 1000000; mStoreDx = 0.01; mStoreMax = 1000;
  mXStored = mYStored = 0;
  mAcc = 1e-8; mH1 = 1e-2; mHmin = 1e-18;
  mX1 = 0; mX2 = 0;

  bContinuous = false; // Single cycle
  bMultix     = true;  // No reasonable storage / transfer mechanism.
}

ODECrawler::~ODECrawler()
{
  delete mXStored;
  delete [] mYStored;
}

/**************************************************************************/

Int_t
ODECrawler::Rkqs(ZVectorD& y, ZVectorD& dydx, Double_t& x, Double_t htry,
		 ZVectorD& yscal, Double_t& hdid, Double_t& hnext)
{
  Double_t errmax,htemp,xnew;

  ZVectorD yerr(mN), ytemp(mN);
  Double_t h=htry;
  while(1) {
    Rkck(y, dydx, x, h, ytemp, yerr);
    errmax = 0.0;
    for(UInt_t i=0; i<mN; i++) errmax = TMath::Max(errmax,TMath::Abs(yerr(i)/yscal(i)));
    errmax /= mAcc;
    if(errmax <= 1.0) break;
    htemp = hSAFETY*h*TMath::Power(errmax,hPSHRNK);
    h = (h >= 0.0 ? TMath::Max(htemp,0.1*h) : TMath::Min(htemp,0.1*h));
    ISdebug(6, GForm("ODECrawler::Rkqs h=%f; errmax=%f", h, errmax));
    xnew = x + h;
    if(xnew == x) {
      ISerr(GForm("ODECrawler::Rkqs stepsize underflow, errmax=%f", errmax));
      return 1;
    }
  }
  if(errmax > hERRCON) hnext = hSAFETY*h*TMath::Power(errmax,hPGROW);
  else hnext=5.0*h;
  x += (hdid=h);
  y = ytemp;
  return 0;
}

/**************************************************************************/

void
ODECrawler::Rkck(ZVectorD& y, ZVectorD& dydx, Double_t x, Double_t h,
		 ZVectorD& yout, ZVectorD& yerr)
{
  static Double_t a2=0.2,a3=0.3,a4=0.6,a5=1.0,a6=0.875,b21=0.2,
    b31=3.0/40.0,b32=9.0/40.0,b41=0.3,b42 = -0.9,b43=1.2,
    b51 = -11.0/54.0, b52=2.5,b53 = -70.0/27.0,b54=35.0/27.0,
    b61=1631.0/55296.0,b62=175.0/512.0,b63=575.0/13824.0,
    b64=44275.0/110592.0,b65=253.0/4096.0,c1=37.0/378.0,
    c3=250.0/621.0,c4=125.0/594.0,c6=512.0/1771.0,
    dc5 = -277.00/14336.0;
  Double_t dc1=c1-2825.0/27648.0,dc3=c3-18575.0/48384.0,
    dc4=c4-13525.0/55296.0,dc6=c6-0.25;

  ZVectorD ak2(mN), ak3(mN), ak4(mN), ak5(mN), ak6(mN), ytemp(mN);

  for(UInt_t i=0; i<mN; i++)
    ytemp(i) = y(i) + b21*h*dydx(i);
  hTrueMaster->ODEDerivatives(x+a2*h,ytemp,ak2);
  for(UInt_t i=0; i<mN; i++)
    ytemp(i)=y(i)+h*(b31*dydx(i)+b32*ak2(i));
  hTrueMaster->ODEDerivatives(x+a3*h,ytemp,ak3);
  for(UInt_t i=0; i<mN; i++)
    ytemp(i)=y(i)+h*(b41*dydx(i)+b42*ak2(i)+b43*ak3(i));
  hTrueMaster->ODEDerivatives(x+a4*h,ytemp,ak4);
  for(UInt_t i=0; i<mN; i++)
    ytemp(i)=y(i)+h*(b51*dydx(i)+b52*ak2(i)+b53*ak3(i)+b54*ak4(i));
  hTrueMaster->ODEDerivatives(x+a5*h,ytemp,ak5);
  for(UInt_t i=0; i<mN; i++)
    ytemp(i)=y(i)+h*(b61*dydx(i)+b62*ak2(i)+b63*ak3(i)+b64*ak4(i)+b65*ak5(i));
  hTrueMaster->ODEDerivatives(x+a6*h,ytemp,ak6);
  for(UInt_t i=0; i<mN; i++)
    yout(i)=y(i)+h*(c1*dydx(i)+c3*ak3(i)+c4*ak4(i)+c6*ak6(i));
  for(UInt_t i=0; i<mN; i++)
    yerr(i)=h*(dc1*dydx(i)+dc3*ak3(i)+dc4*ak4(i)+dc5*ak5(i)+dc6*ak6(i));
}

/**************************************************************************/

void ODECrawler::Crawl()
{
  // Integrate ODE with derivatives provided by mODEMaster from X1 to X2.

  Double_t x, xsav, h, hdid, hnext;

  x = mX1;
  h = TMath::Sign(mH1, mX2-mX1);
  ZVectorD y(mY);
  ZVectorD yscal(mN), dydx(mN);
  if (mStoreMax > 0) xsav = x - 2*mStoreDx;
  for(UInt_t nstp=0; nstp<mMaxSteps; nstp++) {
    hTrueMaster->ODEDerivatives(x, y, dydx);
    ISdebug(6, GForm("ODECrawl Pre Rkqs at x=%f", x));
    for(UInt_t i=0; i<mN; i++) {
      yscal(i)=TMath::Abs(y(i)) + TMath::Abs(dydx(i)*h) + hTINY;
      ISdebug(6, GForm(" %6d %f %f", i, y(i), dydx(i)));
    }

    if(mStoreMax && mStored < mStoreMax-1 && TMath::Abs(x-xsav) > TMath::Abs(mStoreDx)) {
      (*mXStored)(mStored) = x;
      mYStored[mStored] = y;
      mStored++;
      xsav = x;
    }

    if ((x+h-mX2)*(x+h-mX1) > 0.0) h=mX2-x;

    if(Rkqs(y, dydx, x, h, yscal, hdid, hnext)) return;

    ISdebug(6, GForm("ODECrawl Post Rkqs at x=%f hdid=%f hnext=%f",
		     x, hdid, hnext));
    for(UInt_t i=0; i<mN; i++) {
      ISdebug(6, GForm("  %6d %f %f", i, y(i), dydx(i)));
    }

    if(hdid == h) mGuessesOK++; else mGuessesBad++;
    if((x-mX2)*(mX2-mX1) >= 0.0) {
      mY = y;
      if(mStoreMax) {
	(*mXStored)(mStored) = x;
	mYStored[mStored] = y;
	mStored++;
      }
      return;
    }
    if (TMath::Abs(hnext) <= mHmin) {
      ISerr(GForm("ODECrawler::Crawl Step size too small, hnext=%g", hnext));
      return;
    }
    h = hnext;
  }
  ISerr("ODECrawler::Crawl Too many steps ...");
}

/**************************************************************************/

Operator::Arg* ODECrawler::PreDance(Operator::Arg* op_arg)
{
  op_arg = Eventor::PreDance(op_arg);

  hTrueMaster = dynamic_cast<ODECrawlerMaster*>(mODEMaster);
  if(!hTrueMaster) {
    ISerr("ODECrawler::PreDance master not of an ODECrawlerMaster");
    delete op_arg;
    return 0;
  }
  mN = hTrueMaster->ODEOrder();
  if(mY.GetNoElements() != mN) mY.ResizeTo(mN);
  if(!mXStored) {
    mXStored = new ZVector(mStoreMax);
    mYStored = new ZVector[mStoreMax];
  } else if(mXStored->GetNoElements() != mStoreMax) {
    mXStored->ResizeTo(mStoreMax);
    delete [] mYStored; mYStored = new ZVector[mStoreMax];
  } else {
    mXStored->Zero();
  }
  mGuessesOK = mGuessesBad = mStored = 0;
  hTrueMaster->ODEStart(mY, mX1, mX2);

  return op_arg;
}

void
ODECrawler::Operate(Operator::Arg* op_arg) throw(Operator::Exception) { 
  Eventor::PreOperate(op_arg);
  Crawl();
  Eventor::PostOperate(op_arg);
}

/**************************************************************************/

#include "ODECrawler.c7"
