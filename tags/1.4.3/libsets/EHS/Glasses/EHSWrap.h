// $Header$

// Copyright (C) 1999-2005, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef EHS_EHSWrap_H
#define EHS_EHSWrap_H

#include <Glasses/ZNameMap.h>

class EHSWrap : public ZNameMap {
  MAC_RNR_FRIENDS(EHSWrap);

private:
  void _init();

  Bool_t	b_stop_server; //!

protected:
  Int_t		mPort;     // X{GS} 7 Value(-range=>[1,65535,1])

  Bool_t	bServerUp; // X{GS} 7 BoolOut()

public:
  EHSWrap(const Text_t* n="EHSWrap", const Text_t* t=0) :
    ZNameMap(n,t) { _init(); }

  void StartServer(); // X{Ed} 7 MButt(-join=>1)
  void StopServer();  // X{E}  7 MButt()


#include "EHSWrap.h7"
  ClassDef(EHSWrap, 1)
}; // endclass EHSWrap


#endif
