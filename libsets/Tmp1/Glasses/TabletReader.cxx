// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "TabletReader.h"
#include "TabletReader.c7"

#include "TabletStroke.h"

// TabletReader

//______________________________________________________________________________
//
//

ClassImp(TabletReader);

//==============================================================================

void TabletReader::_init()
{
  mMaxX = 1; 
  mPosScale = 0;
  bInvertY = true;

  mButtons = 0;
  mPosX = mPosY = mTouchPosX = mTouchPosY = mPressure = 0;

  bPrintPositions = true;
  bPrintButtons   = true;
  bPrintOther     = false;
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

bool TabletReader::get_report_button(Int_t bb)
{
  bool val = get_button(bb);
  if (bPrintButtons)
  {
    const char* name = get_button_name(bb);
    printf("Button %s %s\n", name, val ? "DOWN" : "UP");
  }
  return val;
}

//==============================================================================

#define WACOMTOOLTYPE_NONE      0x00
#define WACOMTOOLTYPE_PEN       0x01
#define WACOMTOOLTYPE_PENCIL    0x02
#define WACOMTOOLTYPE_BRUSH     0x03
#define WACOMTOOLTYPE_ERASER    0x04
#define WACOMTOOLTYPE_AIRBRUSH  0x05
#define WACOMTOOLTYPE_MOUSE     0x06
#define WACOMTOOLTYPE_LENS      0x07
#define WACOMTOOLTYPE_PAD       0x08
#define WACOMTOOLTYPE_TOUCH	0x09
#define WACOMTOOLTYPE_MAX       0x0A

#define WACOMBUTTON_LEFT        0
#define WACOMBUTTON_MIDDLE      1
#define WACOMBUTTON_RIGHT       2
#define WACOMBUTTON_EXTRA       3
#define WACOMBUTTON_SIDE        4
#define WACOMBUTTON_TOUCH       5
#define WACOMBUTTON_STYLUS      6
#define WACOMBUTTON_STYLUS2     7
#define WACOMBUTTON_BT0         8
#define WACOMBUTTON_BT1         9
#define WACOMBUTTON_BT2         10
#define WACOMBUTTON_BT3         11
#define WACOMBUTTON_BT4         12
#define WACOMBUTTON_BT5         13
#define WACOMBUTTON_BT6         14
#define WACOMBUTTON_BT7         15
#define WACOMBUTTON_BT8         16
#define WACOMBUTTON_BT9         17
#define WACOMBUTTON_BT10        18
#define WACOMBUTTON_BT11        19
#define WACOMBUTTON_BT12        20
#define WACOMBUTTON_BT13        21
#define WACOMBUTTON_BT14        22
#define WACOMBUTTON_BT15        23
#define WACOMBUTTON_BT16        24
#define WACOMBUTTON_BT17        25
#define WACOMBUTTON_BT18        26
#define WACOMBUTTON_BT19        27
#define WACOMBUTTON_BT20        28
#define WACOMBUTTON_BT21        29
#define WACOMBUTTON_BT22        30
#define WACOMBUTTON_BT23        31
#define WACOMBUTTON_MAX         32

#define WACOMFIELD_TOOLTYPE     0
#define WACOMFIELD_SERIAL       1
#define WACOMFIELD_PROXIMITY    2
#define WACOMFIELD_BUTTONS      3
#define WACOMFIELD_POSITION_X   4
#define WACOMFIELD_POSITION_Y   5
#define WACOMFIELD_ROTATION_Z   6
#define WACOMFIELD_DISTANCE	7
#define WACOMFIELD_PRESSURE	8
#define WACOMFIELD_TILT_X       9
#define WACOMFIELD_TILT_Y       10
#define WACOMFIELD_ABSWHEEL     11
#define WACOMFIELD_RELWHEEL     12
#define WACOMFIELD_THROTTLE     13
#define WACOMFIELD_MAX          14

typedef struct
{
	unsigned int valid;        /* Bit mask of WACOMFIELD_xxx bits. */
	int          values[WACOMFIELD_MAX];
} WACOMSTATEMINI;

#ifdef HAS_WAC_GLED
extern "C"
{
  int  WacGledOpen(const char* pszFile, int verbose);
  void WacGledFetchMinMax(WACOMSTATEMINI* mini_state_min, WACOMSTATEMINI* mini_state_max);
  int  WacGledFetch(WACOMSTATEMINI* mini_state);
  void WacGledClose();
  const char* WacGledGetSerialField(unsigned int uField);
  const char* WacGledGetSerialButton(unsigned int uButton);
}
#else
namespace
{
  int  WacGledOpen(const char*, int) { return 1; }
  void WacGledFetchMinMax(WACOMSTATEMINI* mini_state_min, WACOMSTATEMINI* mini_state_max) {}
  int  WacGledFetch(WACOMSTATEMINI* mini_state) { return 1; }
  void WacGledClose() {}
  const char* WacGledGetSerialField(unsigned int uField)   { return "no-shit"; }
  const char* WacGledGetSerialButton(unsigned int uButton) { return "no-shit"; }
}
#endif


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
  printf("MIN x=%d y=%d\n", s_min.values[WACOMFIELD_POSITION_X], s_min.values[WACOMFIELD_POSITION_Y]);
  printf("MAX x=%d y=%d\n", s_max.values[WACOMFIELD_POSITION_X], s_max.values[WACOMFIELD_POSITION_Y]);

  mPosScale = mMaxX / s_max.values[WACOMFIELD_POSITION_X];
  Stamp(FID());

  mButtons = 0;
  mPosX = mPosY = mTouchPosX = mTouchPosY = mPressure = 0;

  TabletStroke *stroke = 0;

  while (!bReqStop)
  {
    if (WacGledFetch(&s))
    {
      printf("OGADUGA -- error fetching ... continuing.\n");
      continue;
    }

    if (s.valid & (1 << WACOMFIELD_BUTTONS))
    {
      Int_t delta = s.values[WACOMFIELD_BUTTONS] ^ mButtons;
      mButtons    = s.values[WACOMFIELD_BUTTONS];

      if (delta & BB_Touch)
      {
	get_report_button(BB_Touch);
      }
      if (delta & BB_Stylus_1)
      {
	get_report_button(BB_Stylus_1);
      }
      if (delta & BB_Stylus_2)
      {
        get_report_button(BB_Stylus_2);
      }
      if (delta & BB_Button_0)
      {
	bool down = get_report_button(BB_Button_0);
	if (down && stroke == 0)
	{
	  stroke = new TabletStroke("Stroke");
	  mQueen->CheckIn(stroke);
	  GLensWriteHolder _wlck(this);
	  Add(stroke);
	}
      }
      if (delta & BB_Button_1)
      {
	bool down = get_report_button(BB_Button_1);
	if (down && stroke)
	{
	  stroke = 0;
	}
      }
    }

    switch (s.values[WACOMFIELD_TOOLTYPE])
    {
      case WACOMTOOLTYPE_PEN:
      {
	Float_t x = mPosScale * s.values[WACOMFIELD_POSITION_X];
	Float_t y = mPosScale * (bInvertY ? s_max.values[WACOMFIELD_POSITION_Y] - s.values[WACOMFIELD_POSITION_Y] : s.values[WACOMFIELD_POSITION_Y]);
	if (bPrintPositions)
	{
	  printf("PEN x=%d y=%d, p=%d, prox=%d\n",
		 s.values[WACOMFIELD_POSITION_X],
		 s.values[WACOMFIELD_POSITION_Y],
		 s.values[WACOMFIELD_PRESSURE],
		 s.values[WACOMFIELD_PROXIMITY]);
	}
	if (stroke && get_button(BB_Touch))
	{
	  GLensReadHolder _lck(stroke);
	  stroke->AddPoint(x, y);
	  stroke->StampReqTring(TabletStroke::FID());
	}
	break;
      }
      case WACOMTOOLTYPE_ERASER:
      {
	if (bPrintPositions)
	{
	  printf("ERS x=%d y=%d, p=%d, prox=%d\n",
		 s.values[WACOMFIELD_POSITION_X],
		 s.values[WACOMFIELD_POSITION_Y],
		 s.values[WACOMFIELD_PRESSURE],
		 s.values[WACOMFIELD_PROXIMITY]);
	}
	break;
      }
      default:
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
	break;
      }
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
