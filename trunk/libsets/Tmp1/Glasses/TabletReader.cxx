// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletReader.h"
#include "TabletStroke.h"
#include "TabletReader.c7"


// TabletReader

//______________________________________________________________________________
//
//

ClassImp(TabletReader);

//==============================================================================

void TabletReader::_init()
{
  bScalePos = true;
  bInvertY = true;
  mScaledW = 1; 
  mPosScale = mPrsScale = 0;
  mOffX = mOffY = 0;

  mButtons = 0;
  bButton0 = bButton1 = bStylus1 = bStylus2 = false;
  bInProximity = bInTouch = false;
  mPenX = mPenY = mPenT = mPenP = 0;

  bPrintButtEvs   = false;
  bPrintButtState = false;
  bPrintPositions = false;
  bPrintOther     = true;

  mMarkSize = 0.05;
  mInTouchColor.rgba(1, 0, 0, 1);
  mInProximityColor.rgba(0, 1, 0, 1);
  mStrokeColor.rgba(1, 0.1, 0.5, 1);
}

TabletReader::TabletReader(const Text_t* n, const Text_t* t) :
  ZNode(n, t),
  mTabletThread(0)
{
  _init();
}

TabletReader::~TabletReader()
{}

//==============================================================================

const char* TabletReader::get_button_name(Int_t bb)
{
  const char* name = "undef";
  switch (bb)
  {
    case BB_Touch:    name = "Touch";    break;
    case BB_Stylus_1: name = "Stylus_1"; break;
    case BB_Stylus_2: name = "Stylus_2"; break;
    case BB_Button_0: name = "Button_0"; break;
    case BB_Button_1: name = "Button_1"; break;
  }
  return name;
}

Bool_t TabletReader::flip_report_button(Int_t bb)
{
  // Bool_t prev = get_button(bb);
  mButtons ^= bb;
  Bool_t val = get_button(bb);
  if (bPrintButtEvs)
  {
    const char* name = get_button_name(bb);
    printf("Button %s %s\n", name, val ? "DOWN" : "UP");
    // printf("Button %s %s, was %s\n", name, val ? "DOWN" : "UP", prev ? "DOWN" : "UP");
  }
  return val;
}

Bool_t TabletReader::check_pen_buttons(Int_t buttons_delta)
{
  // Check for changes in pen-related buttons.
  // Returns true if there was a change.

  Bool_t change = false;

  if (buttons_delta & BB_Touch)
  {
    Bool_t down = flip_report_button(BB_Touch);
    if (down)
      mTouchTime.SetNow();
    bInTouch = down;
    change = true;
  }
  if (buttons_delta & BB_Stylus_1)
  {
    bStylus1 = flip_report_button(BB_Stylus_1);
    change = true;
  }
  if (buttons_delta & BB_Stylus_2)
  {
    bStylus2 = flip_report_button(BB_Stylus_2);
    change = true;
  }

  return change;
}

void TabletReader::clear_pen_buttons()
{
  mButtons &= ~BB_Pad_Buttons;
  bInTouch = bStylus1 = bStylus2 = false;
}

Bool_t TabletReader::check_pad_buttons(Int_t buttons_delta)
{
  // Check for changes in pad-related buttons.
  // Returns true if there was a change.

  Bool_t change = false;

  if (buttons_delta & BB_Button_0)
  {
    bool down = flip_report_button(BB_Button_0);
    if (down && mStroke == 0)
    {
      TabletStroke *stroke = new TabletStroke("Stroke");
      stroke->SetColorByRef(mStrokeColor);
      mQueen->CheckIn(stroke);
      GLensWriteHolder _wlck(this);
      SetStroke(stroke);
      Add(stroke);
    }
    bButton0 = down;
    change = true;
  }
  if (buttons_delta & BB_Button_1)
  {
    bool down = flip_report_button(BB_Button_1);
    if (down && mStroke != 0)
    {
      GLensWriteHolder _wlck(this);
      SetStroke(0);
    }
    bButton1 = down;
    change = true;
  }

  return change;
}

//==============================================================================

#include "Tmp1/TabletWacomDefines.h"

void TabletReader::StartRead()
{
  static const Exc_t _eh("TabletReader::StartRead ");

  {
    GMutexHolder _lck(mTabletMutex);
    if (mTabletThread != 0)
    {
      throw _eh + "Thread already running.";
    }
    mTabletThread = GThread::Self();

    if (WacGledOpen("/dev/input/wacom", 1))
    {
      mTabletThread = 0;
      throw _eh + "Spawning dumper failed.";
    }
  }

  WACOMSTATEMINI s_min, s_max, s;

  WacGledFetchMinMax(&s_min, &s_max);
  printf("MIN x=%d y=%d p=%d\n", s_min.values[WACOMFIELD_POSITION_X], s_min.values[WACOMFIELD_POSITION_Y], s_min.values[WACOMFIELD_PRESSURE]);
  printf("MAX x=%d y=%d p=%d\n", s_max.values[WACOMFIELD_POSITION_X], s_max.values[WACOMFIELD_POSITION_Y], s_max.values[WACOMFIELD_PRESSURE]);

  if (bScalePos)
  {
    mPosScale = mScaledW / s_max.values[WACOMFIELD_POSITION_X];
    mOffX = s_max.values[WACOMFIELD_POSITION_X] / 2;
    mOffY = s_max.values[WACOMFIELD_POSITION_Y] / 2;
  }
  else
  {
    mPosScale = 1;
    mOffX = mOffY = 0;
  }
  mPrsScale = 1.0f / s_max.values[WACOMFIELD_PRESSURE];
  Stamp(FID());

  mButtons = 0;
  bButton0 = bButton1 = bStylus1 = bStylus2 = false;
  bInProximity = bInTouch = false;
  mPenX = mPenY = mPenT = mPenP = 0;

  while (!bReqStop)
  {
    if (WacGledFetch(&s))
    {
      printf("OGADUGA -- error fetching ... continuing.\n");
      continue;
    }

    Bool_t stamp_p = false;

    Int_t  tool          = s.values[WACOMFIELD_TOOLTYPE];
    Int_t  buttons       = s.values[WACOMFIELD_BUTTONS];
    Int_t  buttons_delta = buttons ^ mButtons;
    Bool_t buttons_valid = s.valid & (1 << WACOMFIELD_BUTTONS);
    Bool_t proximity     = s.values[WACOMFIELD_PROXIMITY];

    // printf("t=%d, bv=%d\n", tool, buttons_valid);

    if (tool == T_None)
    {
      // This is tricky ... we get these events both for
      // pad-button-up and for proximity-out.
      // Then, there are also dummy events like this, usually in batches
      // of four, sometimes on proximity-in.
      // It took me a while to figure it out.
      if (buttons_valid && check_pad_buttons(buttons_delta))
      {
	stamp_p = true;
      }
      else if ( ! proximity && bInProximity)
      {
	clear_pen_buttons();
	bInProximity = false;
	stamp_p = true;
      }
    }
    else if (tool == T_Pen || tool == T_Eraser)
    {
      stamp_p = true;

      if ( ! bInProximity)
      {
	bInProximity = true;
      }

      if (buttons_valid)
      {
	check_pen_buttons(buttons_delta);
      }

      mPenT = mTouchTime.TimeUntilNow().ToFloat();
      mPenX = mPosScale * (s.values[WACOMFIELD_POSITION_X] - mOffX);
      mPenY = mPosScale * (s.values[WACOMFIELD_POSITION_Y] - mOffY);
      mPenP = mPrsScale * s.values[WACOMFIELD_PRESSURE];
      if (bInvertY) mPenY = -mPenY;
      if (bPrintPositions)
      {
	printf("PEN x=%d y=%d, p=%d, prox=%d\n",
	       s.values[WACOMFIELD_POSITION_X],
	       s.values[WACOMFIELD_POSITION_Y],
	       s.values[WACOMFIELD_PRESSURE],
	       s.values[WACOMFIELD_PROXIMITY]);
      }
      if (mStroke != 0 && get_button(BB_Touch))
      {
	GLensReadHolder _lck(*mStroke);
	mStroke->AddPoint(mPenX, mPenY, mPenT, mPenP);
	mStroke->StampReqTring(TabletStroke::FID());
      }
    }
    else if (tool == T_Pad)
    {
      if (buttons_valid)
      {
	if (check_pad_buttons(buttons_delta))
	  stamp_p = true;
      }
    }
    else
    {
      if (bPrintOther)
      {
	printf("tool= %d, 0x%x: x=%d y=%d, p=%d, prox=%d\n",
	       s.values[WACOMFIELD_TOOLTYPE], s.values[WACOMFIELD_TOOLTYPE],
	       s.values[WACOMFIELD_POSITION_X],
	       s.values[WACOMFIELD_POSITION_Y],
	       s.values[WACOMFIELD_PRESSURE],
	       s.values[WACOMFIELD_PROXIMITY]);
      }
    }

    if (stamp_p)
    {
      if (bPrintButtState)
      {
	printf("S1:%d/%d, S2:%d/%d, B0:%d/%d, B1:%d/%d\n",
	       bStylus1, get_button(BB_Stylus_1),
	       bStylus2, get_button(BB_Stylus_2),
	       bButton0, get_button(BB_Button_0),
	       bButton1, get_button(BB_Button_1));
      }
      StampReqTring(FID());
    }
  }

  {
    GMutexHolder _lck(mTabletMutex);

    printf("OGADUGA -- closing wacom\n");
    WacGledClose();
    printf("OGADUGA -- wacom closed\n");

    bReqStop = false;
    mTabletThread = 0;
  }
}

void TabletReader::StopRead()
{
  static const Exc_t _eh("TabletReader::StopRead ");

  GMutexHolder _lck(mTabletMutex);

  if (mTabletThread == 0)
    throw _eh + "Thread not running.";
  if (bReqStop)
    throw _eh + "Already requested.";

  bReqStop = true;

  mTabletThread->Kill(GThread::SigINT);
}
