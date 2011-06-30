// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTime.h"
#include <time.h>
#include <sys/time.h>

//______________________________________________________________________
// GTime
//

ClassImp(GTime);

void GTime::SetNow()
{
  struct timeval t;
  gettimeofday(&t, 0);
  mSec   = t.tv_sec;
  mMuSec = t.tv_usec;
}

GTime GTime::TimeUntilNow()
{
  GTime n(I_Now);
  n -= *this;
  return n;
}

/**************************************************************************/

GTime& GTime::operator=(Long64_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this = tt;
  return *this;
}

GTime& GTime::operator=(ULong64_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this = tt;
  return *this;
}

GTime& GTime::operator=(Double_t sec)
{
  Long64_t s = (Long64_t)sec;
  GTime tt(s, 1000000*((Long64_t)(sec-s)) );
  *this = tt;
  return *this;
}

/**************************************************************************/

GTime& GTime::operator+=(const GTime& t)
{
  mSec   += t.mSec;
  mMuSec += t.mMuSec;
  if(mMuSec > 1000000) {
    ++mSec;
    mMuSec -= 1000000;
  }
  return *this;
}

GTime& GTime::operator-=(const GTime& t)
{
  mSec   -= t.mSec;
  mMuSec -= t.mMuSec;
  if(mMuSec < 0) {
    --mSec;
    mMuSec += 1000000;
  }
  return *this;
}

GTime& GTime::operator+=(Long64_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this += tt;
  return *this;
}

GTime& GTime::operator-=(Long64_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this -= tt;
  return *this;
}

GTime& GTime::operator+=(ULong64_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this += tt;
  return *this;
}

GTime& GTime::operator-=(ULong64_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this -= tt;
  return *this;
}

GTime& GTime::operator+=(Double_t sec)
{
  Long64_t s = (Long64_t)sec;
  GTime tt(s, 1000000*((Long64_t)(sec-s)) );
  *this += tt;
  return *this;
}

GTime& GTime::operator-=(Double_t sec)
{
  Long64_t s = (Long64_t)sec;
  GTime tt(s, 1000000*((Long64_t)(sec-s)) );
  *this -= tt;
  return *this;
}

/**************************************************************************/

GTime GTime::operator+(const GTime& t) const
{
  GTime r(*this);
  r += t;
  return r;
}

GTime GTime::operator-(const GTime& t) const
{
  GTime r(*this);
  r -= t;
  return r;
}

/**************************************************************************/

bool GTime::operator<(const GTime& t) const
{
  if(mSec == t.mSec) return mMuSec < t.mMuSec;
  return mSec < t.mSec;
}

bool GTime::operator>(const GTime& t) const
{
  if(mSec == t.mSec) return mMuSec > t.mMuSec;
  return mSec > t.mSec;
}

bool GTime::operator<=(const GTime& t) const
{
  if(mSec == t.mSec) return mMuSec <= t.mMuSec;
  return mSec <= t.mSec;
}

bool GTime::operator>=(const GTime& t) const
{
  if(mSec == t.mSec) return mMuSec >= t.mMuSec;
  return mSec >= t.mSec;
}

bool GTime::operator==(const GTime& t) const
{
  return mMuSec == t.mMuSec && mSec == t.mSec;
}

//==============================================================================

namespace
{
  typedef struct tm* (time_foo_r)(const time_t*, struct tm*);

  TString to_asctime(Long64_t sec, time_foo_r foo, Bool_t show_tz)
  {
    time_t    time(sec);
    struct tm t;
    char      buf[32];
    TString   txt(asctime_r(foo(&time, &t), buf));
    txt.Chop();
    if (show_tz) {
      txt += " ";
      txt += (foo == gmtime_r) ? "UTC" : t.tm_zone;
    }
    return txt;
  }

  TString to_datetime(Long64_t sec, time_foo_r foo, Bool_t show_tz)
  {
    time_t    time(sec);
    struct tm t;
    foo(&time, &t);
    TString txt;
    txt.Form("%d-%02d-%02d %02d:%02d:%02d", 1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
    if (show_tz) {
      txt += " ";
      txt += (foo == gmtime_r) ? "UTC" : t.tm_zone;
    }
    return txt;
  }
}

TString GTime::ToAscUTC(Bool_t show_tz) const
{
  return to_asctime(mSec, gmtime_r, show_tz);
}

TString GTime::ToAscLocal(Bool_t show_tz) const
{
  return to_asctime(mSec, localtime_r, show_tz);
}

TString GTime::ToDateTimeUTC(Bool_t show_tz) const
{
  return to_datetime(mSec, gmtime_r, show_tz);
}

TString GTime::ToDateTimeLocal(Bool_t show_tz) const
{
  return to_datetime(mSec, localtime_r, show_tz);
}

//==============================================================================

void GTime::Sleep()
{
  struct timespec req, rem;
  req.tv_sec  = mSec;
  req.tv_nsec = mMuSec * 1000;
  if (nanosleep(&req, &rem))
    perror("GTime::Sleep");
  // !!! Should sleep on? Need flag.
}

//------------------------------------------------------------------------------

Long64_t GTime::SleepMiliSec(UInt_t ms,
			     Bool_t break_on_signal,
			     Bool_t warn_on_signal)
{
  struct timespec req, rem;
  req.tv_sec  = ms / 1000;
  req.tv_nsec = (ms % 1000) * 1000000;

  while (true)
  {
    if (nanosleep(&req, &rem))
    {
      if (warn_on_signal)
      {
	perror("GTime::SleepMiliSec");
      }
      if (break_on_signal)
      {
	Long64_t remms = 1000ll*rem.tv_sec + rem.tv_nsec / 1000000;
	return remms;
      }
      else
      {
	req = rem;
      }
    }
    else
    {
      return 0;
    }
  }
}
