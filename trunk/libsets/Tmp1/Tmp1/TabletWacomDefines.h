// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#ifndef Tmp1_TabletWacomDefines_H
#define Tmp1_TabletWacomDefines_H

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

#endif
