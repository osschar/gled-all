// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Net1_Ip4AddressLocator_H
#define Net1_Ip4AddressLocator_H

#include <Glasses/ZGlass.h>
#include <Stones/IpAddressLocation.h>

class Ip4AddressLocator : public ZGlass
{
  MAC_RNR_FRIENDS(Ip4AddressLocator);

public:
  typedef vector<UInt_t>     vUInt_t;
  typedef vUInt_t::iterator  vUInt_i;

  typedef vector<IpAddressLocation>  vIpALoc_t;
  typedef vIpALoc_t::iterator        vIpALoc_i;

private:
  void _init();

protected:
  vUInt_t       mIpVec;
  vUInt_t       mLocIdxVec;

  vIpALoc_t     mLocInfoVec;

public:
  Ip4AddressLocator(const Text_t* n="Ip4AddressLocator", const Text_t* t=0);
  virtual ~Ip4AddressLocator();

  void LoadFromCsvFile(const TString& fname); // X{Ed} 7 MCWButt()

  void QueryHostIp(UInt_t ip4);

#include "Ip4AddressLocator.h7"
  ClassDef(Ip4AddressLocator, 1);
}; // endclass Ip4AddressLocator

#endif
