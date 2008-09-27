// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef AliEnViz_AEVMlSucker_H
#define AliEnViz_AEVMlSucker_H

#include <Glasses/ZNameMap.h>

class AEVMlSucker : public ZNameMap
{
  MAC_RNR_FRIENDS(AEVMlSucker);

private:
  void _init();

protected:
  GThread*  mSuckerThread;

  static void* tl_Suck(AEVMlSucker* s);
  void Suck();

public:
  AEVMlSucker(const Text_t* n="AEVMlSucker", const Text_t* t=0) :
    ZNameMap(n,t) { _init(); }
  virtual ~AEVMlSucker() {}

  void StartSucker(); // X{E} 7 MButt()
  void StopSucker();  // X{E} 7 MButt()

#include "AEVMlSucker.h7"
  ClassDef(AEVMlSucker, 1);
}; // endclass AEVMlSucker


#endif
