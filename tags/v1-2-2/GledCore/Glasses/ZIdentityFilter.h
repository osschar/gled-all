// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZIdentityFilter_H
#define GledCore_ZIdentityFilter_H

#include <Glasses/ZMirFilter.h>
#include <Glasses/ZIdentity.h>

class ZIdentityFilter : public ZMirFilter {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZIdentityFilter);

private:
  void _init();

protected:
  ZIdentity*		mIdentity;	// X{GS} L{}
  UChar_t		mOnMatch;	// X{GS} 7 PhonyEnum(-vals=>[ZMirFilter::R_Allow,Allow,ZMirFilter::R_Deny,Deny],-width=>6)

public:
  ZIdentityFilter(const Text_t* n="ZIdentityFilter", const Text_t* t=0) :
    ZMirFilter(n,t) { _init(); }

  virtual Result_e FilterMIR(ZMIR& mir);

#include "ZIdentityFilter.h7"
  ClassDef(ZIdentityFilter, 1)
}; // endclass ZIdentityFilter

GlassIODef(ZIdentityFilter);

#endif
