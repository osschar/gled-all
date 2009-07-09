// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Geom1_ZGlProgram_H
#define Geom1_ZGlProgram_H

#include <Glasses/ZList.h>

#include <Eye/Ray.h>

class ZGlProgram : public ZList
{
  // 7777 RnrCtrl(RnrBits(0,4,0,0, 0,0,0,0))
  MAC_RNR_FRIENDS(ZGlProgram);

public:
  enum PrivRayQN_e 
  {
    PRQN_offset = RayNS::RQN_user_0,
    PRQN_relink,
    PRQN_rebuild
  };

private:
  void _init();

protected:
  Bool_t        bLinked;   //! X{GS} 7 BoolOut();
  TString       mLog;      //! X{S}

public:
  ZGlProgram(const Text_t* n="ZGlProgram", const Text_t* t=0);
  virtual ~ZGlProgram();

  void EmitRelinkRay();    // X{E}  7 MButt()
  void EmitRebuildRay();   // X{E}  7 MButt()
  void ReloadAndRebuild(); // X{ED} 7 MButt()

  void PrintLog(); //! X{E} 7 MButt()

#include "ZGlProgram.h7"
  ClassDef(ZGlProgram, 1);
}; // endclass ZGlProgram

#endif
