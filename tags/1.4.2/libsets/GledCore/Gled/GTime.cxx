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

#define NANO_FAK 1000000000

ClassImp(GTime);

GTime GTime::sApproximateTime(GTime::I_Now);

GTime::GTime(Init_e i)
{
  switch (i)
  {
    case I_Zero:  SetZero();  break;
    case I_Now:   SetNow();   break;
    case I_Never: SetNever(); break;
  }
}

void GTime::canonize()
{
  if (mNSec < 0) {
    Long64_t uf = 1 - mNSec / NANO_FAK;
    mSec  -= uf;
    mNSec += NANO_FAK * uf;
  }
  else if (mNSec > NANO_FAK) {
    Long64_t of = mNSec / NANO_FAK;
    mSec  += of;
    mNSec -= NANO_FAK * of;
  }
}

//==============================================================================

void GTime::SetNow()
{
  struct timeval t;
  gettimeofday(&t, 0);
  mSec  = t.tv_sec;
  mNSec = t.tv_usec * 1000;
}

GTime GTime::TimeUntilNow()
{
  GTime n(I_Now);
  n -= *this;
  return n;
}

void GTime::SetNever()
{
  mSec = mNSec = LLONG_MIN;
}

Bool_t GTime::IsNever() const
{
  return mSec == LLONG_MIN && mNSec == LLONG_MIN;
}

//==============================================================================

GTime& GTime::operator=(Double_t sec)
{
  mSec = (Long64_t) sec;
  if (sec < 0) --mSec;
  mNSec = (Long64_t) (NANO_FAK * (sec - mSec));
  return *this;
}

//==============================================================================

GTime& GTime::operator+=(const GTime& t)
{
  mSec   += t.mSec;
  mNSec += t.mNSec;
  if (mNSec > NANO_FAK) {
    ++mSec;
    mNSec -= NANO_FAK;
  }
  return *this;
}

GTime& GTime::operator-=(const GTime& t)
{
  mSec  -= t.mSec;
  mNSec -= t.mNSec;
  if (mNSec < 0) {
    --mSec;
    mNSec += NANO_FAK;
  }
  return *this;
}

GTime& GTime::operator+=(Double_t sec)
{
  GTime tt(sec);
  *this += tt;
  return *this;
}

GTime& GTime::operator-=(Double_t sec)
{
  GTime tt(sec);
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
  if(mSec == t.mSec) return mNSec < t.mNSec;
  return mSec < t.mSec;
}

bool GTime::operator>(const GTime& t) const
{
  if(mSec == t.mSec) return mNSec > t.mNSec;
  return mSec > t.mSec;
}

bool GTime::operator<=(const GTime& t) const
{
  if(mSec == t.mSec) return mNSec <= t.mNSec;
  return mSec <= t.mSec;
}

bool GTime::operator>=(const GTime& t) const
{
  if(mSec == t.mSec) return mNSec >= t.mNSec;
  return mSec >= t.mSec;
}

bool GTime::operator==(const GTime& t) const
{
  return mNSec == t.mNSec && mSec == t.mSec;
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

  TString to_date(Long64_t sec, time_foo_r foo)
  {
    time_t    time(sec);
    struct tm t;
    foo(&time, &t);
    TString txt;
    txt.Form("%d-%02d-%02d", 1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday);
    return txt;
  }

  TString to_compact_datetime(Long64_t sec, time_foo_r foo)
  {
    time_t    time(sec);
    struct tm t;
    foo(&time, &t);
    return TString::Format("%d%02d%02d-%02d%02d%02d", 1900 + t.tm_year, 1 + t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
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

TString GTime::ToCompactDateTimeUTC() const
{
  return IsNever() ? "Never" : to_compact_datetime(mSec, gmtime_r);
}

TString GTime::ToCompactDateTimeLocal() const
{
  return IsNever() ? "Never" : to_compact_datetime(mSec, localtime_r);
}

TString GTime::ToDateUTC() const
{
  return IsNever() ? "Never" : to_date(mSec, gmtime_r);
}

TString GTime::ToDateLocal() const
{
  return IsNever() ? "Never" : to_date(mSec, localtime_r);
}

TString GTime::ToWebTimeGMT(Bool_t show_tz) const
{
  return IsNever() ? "Never" : to_webtime(mSec, gmtime_r, show_tz);
}

TString GTime::ToWebTimeLocal(Bool_t show_tz) const
{
  return IsNever() ? "Never" : to_webtime(mSec, localtime_r, show_tz);
}

TString GTime::ToHourMinSec(Bool_t force_non_negative) const
{
  // Format as hh:mm:ss, useful for time differences.

  if (IsNever()) return "Never";

  Long64_t osec  = mSec;
  if (force_non_negative && osec < 0) osec = 0;

  Long64_t hours = osec / 3600;
  Int_t    sah   = osec % 3600;
  Int_t    min   = sah / 60;
  Int_t    sec   = sah % 60;
  return TString::Format("%02lld:%02d:%02d", hours, min, sec);    
}

//------------------------------------------------------------------------------

GTime GTime::TimeOfTheDayUTC() const
{
  time_t    time(mSec);
  struct tm t;
  gmtime_r(&time, &t);
  return GTime(3600*t.tm_hour + 60*t.tm_min + t.tm_sec, 0);
}

GTime GTime::TimeOfTheDayLocal() const
{
  time_t    time(mSec);
  struct tm t;
  localtime_r(&time, &t);
  return GTime(3600*t.tm_hour + 60*t.tm_min + t.tm_sec, 0);
 
}

//==============================================================================

void GTime::Sleep()
{
  struct timespec req, rem;
  req.tv_sec  = mSec;
  req.tv_nsec = mNSec;
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

//==============================================================================

const GTime& GTime::ApproximateTime()
{
  // Returns approximate time, rounded to one second.

  return sApproximateTime;
}

GTime GTime::ApproximateFuture(Double_t sec)
{
  // Returns approximate time sec seconds in the future.

  return sApproximateTime + GTime(sec);
}

GTime GTime::ApproximateFuture(Long64_t sec)
{
  // Returns approximate time sec seconds in the future.

  return sApproximateTime + GTime(sec, 0);
}

GTime GTime::ApproximateFuture(Int_t sec)
{
  // Returns approximate time sec seconds in the future.

  return sApproximateTime + GTime(sec, 0);
}

Long64_t GTime::UpdateApproximateTime(const GTime& now)
{
  if (now.mNSec > 500000000)
    sApproximateTime.mSec = now.mSec + 1;
  else
    sApproximateTime.mSec = now.mSec;

  return sApproximateTime.mSec;
}

//==============================================================================

void GTime::NetStreamer(TBuffer& b)
{
  if (b.IsReading())
  {
    b >> mSec >> mNSec;
  }
  else
  {
    b << mSec << mNSec;
  }
}
