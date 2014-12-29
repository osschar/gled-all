// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef RootGeo_ZGeoOvl_H
#define RootGeo_ZGeoOvl_H

#include <Glasses/ZGeoNode.h>

class ZGeoOvl : public ZGeoNode {

  // 7777 RnrCtrl(RnrBits(2,4,6,0, 0,0,0,3))
  MAC_RNR_FRIENDS(ZGeoOvl);
  friend class ZGeoOvlMgr;

private:
  void _init();

protected:
  Bool_t    mIsExtr;    // X{GS}
  Double_t  mOverlap;   // X{GS}  7 ValOut()
  Int_t     mPM_N;      // X{GS}
  Float_t*  mPM_p;      //[mPM_N*3] X{gS}
  ZColor    mPM_Col;    // X{GSP}
  Bool_t    mRnrMark;   // X{GS}  7 Bool(-join=>1)
  Bool_t    mRnrNode;   // X{GS}  7 Bool()

public:
  ZGeoOvl(const Text_t* n="ZGeoOvl", const Text_t* t=0) : ZGeoNode(n,t)
  { _init(); }

  virtual void Restore(TGeoVolume* vol);

  void DumpOvl(); //! X{E} 7 MButt()

#include "ZGeoOvl.h7"
  ClassDef(ZGeoOvl, 1);
}; // endclass ZGeoOvl


#endif
