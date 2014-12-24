/*
  To build (I gave up on Makefile.am, see how wacdump.o is built):
  # configure linuxwacom as: ./configure CFLAGS="-fPIC -O2 -g"
  gcc -DHAVE_CONFIG_H -I. -I../../src/include -fPIC -Wall  -g -O2 -D__amd64__ -c -o wacdumpgled.o wacdumpgled.c
  ar -r libWacDumpGled.a wacdumpgled.o wactablet.o wacserial.o wacusb.o

  You also need sth like this to prevent xinput from eating the events:
  xinput list | grep Wacom | perl -ne 'if(/id=(\d+)/) { `xinput set-prop $1 121 0`; }'
 */

/*****************************************************************************
** wacdumpgled.c -- Matevz Tadel, Jun 2010
**
** Strip-off hack of wacdump.c from linuxwacom, which is:
**
** Copyright (C) 2002 - 2008 - John E. Joganic and Ping Cheng
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
**
** REVISION HISTORY
**   2002-12-17 0.3.3 - split ncurses from main file to avoid namespace
**                      collision in linux/input.h
**   2002-12-21 0.3.4 - changed to FILE* to file descriptors
**   2003-01-01 0.3.5 - moved refresh to start display immediately
**   2003-01-25 0.3.6 - moved usb code to wacusb.c
**   2003-01-26 0.3.7 - applied Dean Townsley's Acer C100 patch
**   2003-01-27 0.3.8 - added logging, better error recovery
**   2003-01-31 0.5.0 - new release
**   2003-02-12 0.5.1 - added version option and fixed usage
**   2003-06-19 0.5.2 - added patch for I2 6x8 id 0x47
**   2005-02-17 0.6.6 - added I3 and support 64-bit system
**   2005-09-01 0.7.0 - Added Cintiq 21UX
**   2005-02-17 0.7.1 - added Graphire 4
**   2006-02-27 0.7.3 - added DTF 521, I3 12x12 & 12x19
**   2006-05-05 0.7.4 - Removed older 2.6 kernels
**   2008-12-31 0.8.2 - Support USB Tabket PCs
**
****************************************************************************/

#include "../include/util-config.h"

#include "wactablet.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <signal.h>

#define WACDUMP_VER "0.8.2"

/* from linux/input.h */
#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define BIT(x)  (1UL<<((x)%BITS_PER_LONG))
#define LONG(x) ((x)/BITS_PER_LONG)
#define ISBITSET(x,y) ((x)[LONG(y)] & BIT(y))

typedef struct
{
  unsigned int valid;        /* Bit mask of WACOMFIELD_xxx bits. */
  int          values[WACOMFIELD_MAX];
} WACOMSTATEMINI;


/******************************************************************************/

static int InitTablet(WACOMTABLET hTablet, int verbose)
{
  int nMajor, nMinor, nRelease;
  WACOMMODEL model;
  const char* pszName;
  const char* pszClass = "UNK_CLS";
  const char* pszVendor = "UNK_VNDR";
  const char* pszDevice = "UNK_DEV";
  const char* pszSub = "UNK_SUB";

  /* get model */
  model = WacomGetModel(hTablet);
  pszVendor = WacomGetVendorName(hTablet);
  pszClass = WacomGetClassName(hTablet);
  pszDevice = WacomGetDeviceName(hTablet);
  pszSub = WacomGetSubTypeName(hTablet);
  pszName = WacomGetModelName(hTablet);
  WacomGetROMVersion(hTablet,&nMajor,&nMinor,&nRelease);

  if (verbose)
  {
    printf("Name: '%s', Class: '%s', Version: %d.%d.%d\n", pszName, pszClass, nMajor, nMinor, nRelease);
  }

  return 0;
}

static int FetchTablet(WACOMTABLET hTablet, WACOMSTATEMINI* mstate)
{
  static unsigned char uchBuf[64];
  static int i, nLength, nErrors=0;
  static WACOMSTATE state = WACOMSTATE_INIT;

  nLength = WacomReadRaw(hTablet,uchBuf,sizeof(uchBuf));
  if (nLength < 0)
  {
    fprintf(stderr,"WacomReadRaw: len=%d (nErrors=%d)\n", nLength,++nErrors);
    return 1;
  }

  if ((i=WacomParseData(hTablet,uchBuf,nLength,&state)))
  {
    fprintf(stderr, "WacomParseData: %d, '%s' (nErrors=%d)\n", i,strerror(errno),++nErrors);
    return 1;
  }

  mstate->valid = state.uValid;
  for (i = 0; i < WACOMFIELD_MAX; ++i)
  {
    mstate->values[i] = state.values[i].nValue;
  }

  return 0;
}

/******************************************************************************/

/*
  Public API:
   extern int  WacGledOpen(const char* pszFile, int verbose);
   extern void WacGledFetchMinMax(WACOMSTATEMINI* mini_state_min, WACOMSTATEMINI* mini_state_max);
   extern int  WacGledFetch(WACOMSTATEMINI* mini_state);
   extern void WacGledClose();

  Helpers: 
   extern const char* WacGledGetSerialField(unsigned int uField);
   extern const char* WacGledGetSerialButton(unsigned int uButton)
 */

static WACOMENGINE hEngine = NULL;
static WACOMTABLET hTablet = NULL;

int WacGledOpen(const char* pszFile, int verbose)
{
  WACOMMODEL model = { 0 };

  /* open engine */
  hEngine = WacomInitEngine();
  if (!hEngine)
  {
    perror("failed to open tablet engine");
    return 1;
  }

  /* open tablet */
  hTablet = WacomOpenTablet(hEngine,pszFile,&model);
  if (!hTablet)
  {
    perror("WacomOpenTablet");
    WacomTermEngine(hEngine);
    hEngine = 0;
    return 1;
  }

  /* get device capabilities, build screen */
  if (InitTablet(hTablet, verbose))
  {
    perror("InitTablet");
    WacomCloseTablet(hTablet);
    WacomTermEngine(hEngine);
    hTablet = 0;
    hEngine = 0;
    return 1;
  }

  return 0;
}

void WacGledFetchMinMax(WACOMSTATEMINI* mini_state_min, WACOMSTATEMINI* mini_state_max)
{
  int i, nCaps;
  WACOMSTATE ranges = WACOMSTATE_INIT;

  /* get event types supported, ranges, and immediate values */
  nCaps = WacomGetCapabilities(hTablet);
  WacomGetState(hTablet,&ranges);

  mini_state_min->valid = nCaps;
  mini_state_max->valid = nCaps;
  for (i=0; i<31; ++i)
  {
    if (nCaps & (1<<i))
    {
      mini_state_min->values[i] = ranges.values[i].nMin;
      mini_state_max->values[i] = ranges.values[i].nMax;
    }
  }
}

int WacGledFetch(WACOMSTATEMINI* mini_state)
{
  return FetchTablet(hTablet, mini_state);
}

void WacGledClose()
{
  /* close device */
  WacomCloseTablet(hTablet);
  WacomTermEngine(hEngine);
  hTablet = 0;
  hEngine = 0;
}

/******************************************************************************/

const char* WacGledGetSerialField(unsigned int uField)
{
  static const char* xszField[WACOMFIELD_MAX] =
    {
      "TOOLTYPE", "SERIAL", "IN_PROX", "BUTTON",
      "POS_X", "POS_Y", "ROT_Z", "DISTANCE",
      "PRESSURE", "TILT_X", "TILT_Y", "ABSWHEEL",
      "RELWHEEL", "THROTTLE"
    };

  return (uField >= WACOMFIELD_MAX) ?  "FIELD?" : xszField[uField];
}

const char* WacGledGetSerialButton(unsigned int uButton)
{
  static const char* xszButton[WACOMBUTTON_MAX] =
    {
      "LEFT", "MIDDLE", "RIGHT", "EXTRA", "SIDE",
      "TOUCH", "STYLUS", "STYLUS2", "BT0", "BT1",
      "BT2", "BT3", "BT4", "BT5", "BT6", "BT7",
      "BT8", "BT9", "BT10", "BT11", "BT12", "BT13",
      "BT14", "BT15", "BT16", "BT17", "BT18", "BT19",
      "BT20", "BT21", "BT22", "BT23"
    };

  return (uButton >= WACOMBUTTON_MAX) ?  "FIELD?" : xszButton[uButton];
}
