// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTime.h"
#include <climits>
#include <ctime>
#include <sys/time.h>

//______________________________________________________________________
//

ClassImp(GTime);


GTime::GTime(Init_e i)
{
  switch (i)
  {
    case I_Zero:  SetZero();  break;
    case I_Now:   SetNow();   break;
    case I_Never: SetNever(); break;
  }
}

//==============================================================================

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

void GTime::SetNever()
{
  mSec = mMuSec = LLONG_MIN;
}

Bool_t GTime::IsNever() const
{
  return mSec == LLONG_MIN && mMuSec == LLONG_MIN;
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
  Long64_t s = (Long64_t) sec;
  GTime tt(s, (Long64_t) (1000000.0*(sec-s)));
  *this += tt;
  return *this;
}

GTime& GTime::operator-=(Double_t sec)
{
  Long64_t s = (Long64_t) sec;
  GTime tt(s, (Long64_t) (1000000.0*(sec-s)));
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

  TString to_webtime(Long64_t sec, time_foo_r foo, Bool_t show_tz)
  {
    static const char *day_names[] = { "Sun", "Mon", "Tue", "Wed",  "Thu",  "Fri", "Sat" };
    static const char *month_names[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
				    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

    time_t    time(sec);
    struct tm t;
    foo(&time, &t);
    TString txt;
    txt.Form("%s, %d %s %d %02d:%02d:%02d", day_names[t.tm_wday], t.tm_mday,
	     month_names[t.tm_mon], 1900 + t.tm_year, t.tm_hour, t.tm_min, t.tm_sec);
    if (show_tz) {
      txt += " ";
      txt += (foo == gmtime_r) ? "GMT" : t.tm_zone;
    }
    return txt;
  }
}

TString GTime::ToAscUTC(Bool_t show_tz) const
{
  return IsNever() ? "Never" : to_asctime(mSec, gmtime_r, show_tz);
}

TString GTime::ToAscLocal(Bool_t show_tz) const
{
  return IsNever() ? "Never" : to_asctime(mSec, localtime_r, show_tz);
}

TString GTime::ToDateTimeUTC(Bool_t show_tz) const
{
  return IsNever() ? "Never" : to_datetime(mSec, gmtime_r, show_tz);
}

TString GTime::ToDateTimeLocal(Bool_t show_tz) const
{
  return IsNever() ? "Never" : to_datetime(mSec, localtime_r, show_tz);
}

TString GTime::ToWebTimeGMT(Bool_t show_tz) const
{
  return IsNever() ? "Never" : to_webtime(mSec, gmtime_r, show_tz);
}

TString GTime::ToWebTimeLocal(Bool_t show_tz) const
{
  return IsNever() ? "Never" : to_webtime(mSec, localtime_r, show_tz);
}

TString GTime::ToHourMinSec() const
{
  // Format as hh:mm:ss, useful for time differences.

  if (IsNever()) return "Never";

  Long64_t hours = mSec / 3600;
  Int_t    sah   = mSec % 3600;
  Int_t    min   = sah / 60;
  Int_t    sec   = sah % 60;
  return TString::Format("%02lld:%02d:%02d", hours, min, sec);    

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
