// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef GledCore_ZMEESelfFilter_H
#define GledCore_ZMEESelfFilter_H

#include <Glasses/ZMirFilter.h>

class ZMEESelfFilter : public ZMirFilter {
  // 7777 RnrCtrl("false, 0, RnrBits()")
  MAC_RNR_FRIENDS(ZMEESelfFilter);

private:
  void _init();

protected:

public:
  ZMEESelfFilter(const Text_t* n="ZMEESelfFilter", const Text_t* t=0) :
    ZMirFilter(n,t) { _init(); }

  virtual Result_e FilterMIR(ZMIR& mir);

#include "ZMEESelfFilter.h7"
  ClassDef(ZMEESelfFilter, 1)
}; // endclass ZMEESelfFilter

GlassIODef(ZMEESelfFilter);

#endif
