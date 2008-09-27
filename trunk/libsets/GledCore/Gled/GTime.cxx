// $Header$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTime.h"
#include <time.h>
#include <sys/time.h>

//______________________________________________________________________
// GTime
//

ClassImp(GTime)

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

GTime& GTime::operator=(Long_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this = tt;
  return *this;
}

GTime& GTime::operator=(ULong_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this = tt;
  return *this;
}

GTime& GTime::operator=(Double_t sec)
{
  Long_t s = (Long_t)sec;
  GTime tt(s, 1000000*((Long_t)(sec-s)) );
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

GTime& GTime::operator+=(Long_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this += tt;
  return *this;
}

GTime& GTime::operator-=(Long_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this -= tt;
  return *this;
}

GTime& GTime::operator+=(ULong_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this += tt;
  return *this;
}

GTime& GTime::operator-=(ULong_t mus)
{
  GTime tt(mus/1000000, mus%1000000);
  *this -= tt;
  return *this;
}

GTime& GTime::operator+=(Double_t sec)
{
  Long_t s = (Long_t)sec;
  GTime tt(s, 1000000*((Long_t)(sec-s)) );
  *this += tt;
  return *this;
}

GTime& GTime::operator-=(Double_t sec)
{
  Long_t s = (Long_t)sec;
  GTime tt(s, 1000000*((Long_t)(sec-s)) );
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

/**************************************************************************/

void GTime::Sleep()
{
  struct timespec req, rem;
  req.tv_sec  = mSec;
  req.tv_nsec = mMuSec * 1000;
  if(nanosleep(&req, &rem))
    perror("GTime::Sleep");
  // !!! Should sleep on? Need flag.
}

/**************************************************************************/

void GTime::SleepMiliSec(UInt_t ms)
{
  struct timespec req, rem;
  req.tv_sec  = ms / 1000;
  req.tv_nsec = (ms % 1000) * 1000000;
  if(nanosleep(&req, &rem))
    perror("GTime::SleepMiliSec");
  // !!! Should sleep on? Need flag.
}
