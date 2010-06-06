// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TabletReader_H
#define Tmp1_TabletReader_H

#include <Glasses/ZNode.h>

#include <Gled/GMutex.h>
#include <Gled/GThread.h>

class TabletReader : public ZNode
{
  MAC_RNR_FRIENDS(TabletReader);

public:
  enum ButtonBits_e
  {
    BB_Touch    = 1 << 5,
    BB_Stylus_1 = 1 << 6,
    BB_Stylus_2 = 1 << 7,
    BB_Button_0 = 1 << 8,
    BB_Button_1 = 1 << 9
  };

  enum Tool_e
  {
    T_Pen    = 1,
    T_Eraser = 4
  };

  enum State_e
  {
    S_None, S_Stroke
  };

private:
  void _init();

protected:
  Float_t   mMaxX;     //  X{GS} 7 Value(-range=>[0, 1000, 1, 100])
  Float_t   mPosScale; //! X{G}  7 ValOut()
  Bool_t    bInvertY;  //  X{GS} 7 Bool()

  Int_t     mButtons;
  Float_t   mPosX;
  Float_t   mPosY;
  Float_t   mTouchPosX;
  Float_t   mTouchPosY;
  Float_t   mPressure;

  Bool_t    bPrintPositions;  // X{GS} 7 Bool()
  Bool_t    bPrintButtons;    // X{GS} 7 Bool()
  Bool_t    bPrintOther;      // X{GS} 7 Bool()

  GMutex    mTabletMutex;  //!
  GThread  *mTabletThread; //!
  Bool_t    bReqStop;      //!

  bool get_button(Int_t bb) { return bb && mButtons; }

  const char* get_button_name(Int_t bb);
  bool        get_report_button(Int_t bb);

public:
  TabletReader(const Text_t* n="TabletReader", const Text_t* t=0);
  virtual ~TabletReader();

  void StartRead(); // X{ED} 7 MButt()
  void StopRead();  // X{E}  7 MButt()

#include "TabletReader.h7"
  ClassDef(TabletReader, 1);
}; // endclass TabletReader

#endif
