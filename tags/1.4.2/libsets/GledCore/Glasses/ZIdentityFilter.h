// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZIdentityFilter_H
#define GledCore_ZIdentityFilter_H

#include <Glasses/ZMirFilter.h>
class ZIdentity;

class ZIdentityFilter : public ZMirFilter
{
  MAC_RNR_FRIENDS(ZIdentityFilter);

private:
  void _init();

protected:
  ZLink<ZIdentity>	mIdentity;	// X{gS} L{}
  UChar_t		mOnMatch;	// X{gS} 7 PhonyEnum(-type=>ZMirFilter::Result_e, -names=>[R_Allow,R_Deny], -width=>6)

public:
  ZIdentityFilter(const Text_t* n="ZIdentityFilter", const Text_t* t=0) :
    ZMirFilter(n,t) { _init(); }

  virtual Result_e FilterMIR(ZMIR& mir);

#include "ZIdentityFilter.h7"
  ClassDef(ZIdentityFilter, 1);
}; // endclass ZIdentityFilter


#endif
