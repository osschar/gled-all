// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Net1_IpAddressLocation_H
#define Net1_IpAddressLocation_H

#include <Rtypes.h>
#include <TString.h>

class IpAddressLocation
{
protected:
  TString  mCity;       // X{GR}
  TString  mState;      // X{GR}
  // TString  mCountry;    // X{GR}
  Double_t mLatitude;   // X{GR}
  Double_t mLongitude;  // X{GR}

public:
  IpAddressLocation() : mLatitude(0), mLongitude(0) {}
  IpAddressLocation(const TString& loc);
  virtual ~IpAddressLocation();

#include "IpAddressLocation.h7"
  ClassDefNV(IpAddressLocation, 1);
}; // endclass IpAddressLocation

#endif
