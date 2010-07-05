// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TabletReader_H
#define Tmp1_TabletReader_H

#include <Glasses/ZNode.h>

#include <Gled/GMutex.h>
#include <Gled/GThread.h>
#include <Gled/GTime.h>

class TabletStroke;
class TabletStrokeList;

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
    BB_Button_1 = 1 << 9,
    BB_Pad_Buttons = BB_Touch | BB_Stylus_1 | BB_Stylus_2
  };

  enum Tool_e
  {
    T_None   = 0,
    T_Pen    = 1,
    T_Eraser = 4,
    T_Pad    = 8
  };

  enum StrokeType_e
  {
    SS_None, SS_Absolute, SS_Relative, SS_Continuous
  };

private:
  void _init();

protected:
  StrokeType_e  mStrokeType; //  X{GS} 7 PhonyEnum()
  Bool_t        bInvertY;    //  X{GS} 7 Bool()
  Bool_t        bKeepStrokeInProximity; // X{GS} 7 Bool()

  Float_t       mPosScale;   //! X{G}  7 ValOut(-join=>1)
  Float_t       mPrsScale;   //! X{G}  7 ValOut()
  Int_t         mOffX;       //! X{G}  7 ValOut(-join=>1)
  Int_t         mOffY;       //! X{G}  7 ValOut()

  Int_t         mButtons;
  // The following four are for debug / light-show purposes.
  Bool_t        bButton0;         //! X{G}  7 BoolOut(-join=>1)
  Bool_t        bButton1;         //! X{G}  7 BoolOut()
  Bool_t        bStylus1;         //! X{G}  7 BoolOut(-join=>1)
  Bool_t        bStylus2;         //! X{G}  7 BoolOut()
  Bool_t        bInProximity;     //! X{G}  7 BoolOut(-join=>1)
  Bool_t        bInTouch;         //! X{G}  7 BoolOut()
  Bool_t        bInStroke;        //! X{G}  7 BoolOut()
  // Offsets for multi-touch strokes.
  Float_t       mPenXOff;
  Float_t       mPenYOff;
  Float_t       mPenTOff;

  Float_t       mPenX, mPenY, mPenP, mPenT; //!
  Int_t         mRawX, mRawY, mRawP; //!

  GTime         mEventTime;        //!
  GTime         mFirstStrokeStart; //!
  GTime         mStrokeStart;      //!

  Bool_t        bPrintButtEvs;    // X{GS} 7 Bool(-join=>1)
  Bool_t        bPrintButtState;  // X{GS} 7 Bool()
  Bool_t        bPrintPositions;  // X{GS} 7 Bool(-join=>1)
  Bool_t        bPrintOther;      // X{GS} 7 Bool()

  ZLink<TabletStrokeList> mStrokeList;  // X{GS} L{}
  ZLink<TabletStroke>     mStroke;      // X{GS} L{}

  GMutex    mTabletMutex;  //!
  GThread  *mTabletThread; //!
  Bool_t    bReqStop;      //!

  Bool_t      get_button(Int_t bb) { return bb & mButtons; }

  const char* get_button_name(Int_t bb);
  Bool_t      flip_report_button(Int_t bb);

  Bool_t      check_pen_buttons(Int_t buttons_delta);
  void        clear_pen_buttons();
  Bool_t      check_pad_buttons(Int_t buttons_delta);

  void begin_stroke_list();
  void end_stroke_list();
  void begin_stroke();
  void end_stroke();

public:
  TabletReader(const Text_t* n="TabletReader", const Text_t* t=0);
  virtual ~TabletReader();

  void StartRead(); // X{ED} 7 MButt()
  void StopRead();  // X{E}  7 MButt()

#include "TabletReader.h7"
  ClassDef(TabletReader, 1);
}; // endclass TabletReader

#endif
