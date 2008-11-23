// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Var1_TabletImage_H
#define Var1_TabletImage_H

#include <Glasses/ZImage.h>

class TabletImage : public ZImage
{
  // 7777 RnrCtrl(RnrBits(0,4,0,0))
  MAC_RNR_FRIENDS(TabletImage);

public:
  struct TouchInfo {
    TimeStamp_t fTime;
    Int_t       fX, fY;
    Int_t       fP;

    TouchInfo(TimeStamp_t t=0, Int_t x=0, Int_t y=0, Int_t p=1) :
      fTime(t), fX(x), fY(y), fP(p) {}
  };

private:
  void _init();

protected:
  TimeStamp_t   mTouchStamp; //!
  TouchInfo     mTouch;      //! Should be a list.

  ZLink<ZImage> mBrush;      // X{GS} L{}

public:
  TabletImage(const Text_t* n="TabletImage", const Text_t* t=0) :
    ZImage(n,t) { _init(); }

  virtual void SetStamps(TimeStamp_t s)
  { ZImage::SetStamps(s); mTouchStamp = s; }

  void Touch(Int_t x=0, Int_t y=0, Int_t p=1); // X{E} 7 MCWButt()

#include "TabletImage.h7"
  ClassDef(TabletImage, 1);
}; // endclass TabletImage


#endif
