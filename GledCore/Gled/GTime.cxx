// $Header$

// Copyright (C) 1999-2003, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GTime.h"
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
  GTime n(Now);
  n -= *this;
  return n;
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

GTime& GTime::operator+=(Long_t t)
{
  GTime tt(t/1000000, t%1000000);
  *this += tt;
  return *this;
}

GTime& GTime::operator-=(Long_t t)
{
  GTime tt(t/1000000, t%1000000);
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
