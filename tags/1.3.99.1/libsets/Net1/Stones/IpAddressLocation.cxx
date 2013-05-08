// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "IpAddressLocation.h"

#include "TPRegexp.h"
#include "TUri.h"


// IpAddressLocation

//______________________________________________________________________________
//
//

ClassImp(IpAddressLocation);

//==============================================================================

IpAddressLocation::IpAddressLocation(const TString& loc) :
  mCity(loc)
{
  static const Exc_t _eh("IpAddressLocation::IpAddressLocation ");

  static TPMERegexp comma(",", "o");
  static TPMERegexp comma_space(", ", "o");
  static GMutex     re_mutex;

  GMutexHolder _lck(re_mutex);

  if (comma.Split(loc) != 3)
    throw _eh + "Argument of wrong format passed: '" + loc + "'.";

  TString name = TUri::PctDecode(comma[0]);
  mLatitude  = comma[1].Atof();
  mLongitude = comma[2].Atof();

  // printf("Hello! %s %f %f\n", name.Data(), mLatitude, mLongitude);

  if (comma_space.Split(name) == 2)
  {
    mCity  = comma_space[0];
    mState = comma_space[1];
  }
  else
  {
    mCity = name;
  }
}

IpAddressLocation::~IpAddressLocation()
{}

//==============================================================================
