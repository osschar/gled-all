// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "Moonraker.h"
#include "Moonraker.c7"

#include <TMath.h>

//________________________________________________________________________
//
// An example for ODECrawler.
// Integrates movement of a shell fired from Earth in Earth-Moon system.
// Units: distance ~ earth radius; time ~ day
//	  mass ~ earth mass
//	  V0 = 1 means escape velocity from earth
//________________________________________________________________________

ClassImp(Moonraker);

void Moonraker::_init()
{
  mODECrawler = 0;

  Double_t earth_mass = 5.9736e24;
  Double_t kappa      = 6.673e-11;
  Double_t earth_r    = 6378e3;
  Double_t day        = 24*3600;
  // Double_t em_dist = 383200e3;
  // Double_t month   = 656*3600; // 27 + 1/3 days

  hKappa = kappa*earth_mass*day*day/earth_r/earth_r/earth_r;
  hEscapeVelocity = TMath::Sqrt(2*hKappa);
  mT0Moon = 27.3333; mRMoon = 0.2725; mMMoon = 0.0123; mDMoon = 60.0815;
  mV0 = 1;
  // cout <<"Kappa="<< hKappa <<"\tEscVel="<< hEscapeVelocity <<endl;
  mT = 0; mT0 = 0; mT1 = 2;
  mLat = mLon = mTheta = mPhi = 0;
  mV0 = 1;

  mScale = 1; mLOD = 20;
  mEColor.rgba(0.2, 0.6, 0.8);
  mMColor.rgba(0.2, 1, 0.2);
  mSColor.rgba(1,0.5,0.5); mRShell = 0.1;
  mTColor.rgba(1,1,1);	   mTWidth = 1;
}

void Moonraker::MoonPos(Double_t* r, Double_t t) const
{
  r[0] = mDMoon*TMath::Cos(2*TMath::Pi()/mT0Moon*t);
  r[1] = mDMoon*TMath::Sin(2*TMath::Pi()/mT0Moon*t);
  r[2] = 0;
}

void Moonraker::MoonPos(HPointD& r, Double_t t) const
{
  r.Set(mDMoon*TMath::Cos(2*TMath::Pi()/mT0Moon*t),
	mDMoon*TMath::Sin(2*TMath::Pi()/mT0Moon*t),
	0);
}

/**************************************************************************/

void Moonraker::ODEStart(Double_t y[], Double_t& x1, Double_t& x2)
{
  Double_t phi = 2*TMath::Pi()*mT0 + mLon*TMath::DegToRad(),
         theta = mLat*TMath::DegToRad();
  y[0] = TMath::Cos(phi)*TMath::Cos(theta);
  y[1] = TMath::Sin(phi)*TMath::Cos(theta);
  y[2] = TMath::Sin(theta);

  ZPoint z;
  for (Int_t i=0; i<3; i++) z[i] = mV0*hEscapeVelocity*y[i];
  ZTrans t;
  t.SetRotByAngles(mPhi*TMath::DegToRad(), mTheta*TMath::DegToRad(), 0);
  t.RotateIP(z);
  for (Int_t i=0; i<3; i++) y[i + 3] = z[i];

  x1 = mT0; x2 = mT1;
}

namespace
{
  inline Double_t p223(Double_t x) { return x*TMath::Sqrt(x); }
}

void
Moonraker::ODEDerivatives(Double_t x, const Double_t y[], Double_t d[])
{
  HPointD pos(y);
  HPointD moon; MoonPos(moon, x);
  HPointD md(pos);
  md -= moon;
  Double_t r0 = p223(pos.Mag2());
  Double_t r1 = p223(md.Mag2());
  for (Int_t i=0; i<3; i++)
  {
    d[i]   = y[i+3];
    d[i+3] = -hKappa*(pos[i]/r0 + md[i]/r1*mMMoon);
  }
}
