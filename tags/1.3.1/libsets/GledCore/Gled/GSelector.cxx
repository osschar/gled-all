// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GSelector.h"

#include <TSocket.h>

#include <sys/select.h>
// #include <sys/time.h>
// #include <sys/types.h>
// #include <unistd.h>
#include <errno.h>

void GFdSet::Add(void* ud, Int_t fd)
{
  insert(make_pair(ud, fd));
}

void GFdSet::Add(TSocket* s)
{
  insert(make_pair(s, s->GetDescriptor()));
}

void GFdSet::Remove(void* ud)
{
  erase(ud);
}

//==============================================================================

ClassImp(GSelector);

//------------------------------------------------------------------------------

GSelector::GSelector(Init_e e) : GMutex(e)
{}

GSelector::~GSelector()
{}

//------------------------------------------------------------------------------

Int_t GSelector::Select()
{
  // Prepare sets.
  int Mfd = 0;

  fd_set read, write, except;

  FD_ZERO(&read);   FD_ZERO(&write);   FD_ZERO(&except);
  fReadOut.clear(); fWriteOut.clear(); fExceptOut.clear();
  Lock();
  for (GFdSet_i i=fRead.begin(); i!=fRead.end(); ++i)
  {
    FD_SET(i->second, &read);
    if (i->second > Mfd)
      Mfd = i->second;
  }
  for (GFdSet_i i=fWrite.begin(); i!=fWrite.end(); ++i)
  {
    FD_SET(i->second, &write);
    if (i->second > Mfd)
      Mfd = i->second;
  }
  for (GFdSet_i i=fExcept.begin(); i!=fExcept.end(); ++i)
  {
    FD_SET(i->second, &except);
    if (i->second > Mfd)
      Mfd = i->second;
  }
  Unlock();

  errno = 0;
  int ret;
  if (fTimeOut <= 0)
  {
    ret = select(Mfd+1, &read, &write, &except, 0);
  }
  else
  {
    struct timeval timeout;
    timeout.tv_sec = (time_t) fTimeOut;
    timeout.tv_usec = (time_t)(1000000*(fTimeOut - timeout.tv_sec));
    ret = select(Mfd+1, &read, &write, &except, &timeout);
  }

  if (ret == -1)
  {
    switch(errno)
    {
      case 0: // Cancelled ... or sth ...
	fError = SE_Null;
	return -1;
      case EBADF:
	fError = SE_BadFD;
	ISerr("GSelector::Bad FileDescriptor");
	return -1;
      case EINTR:
	fError = SE_Interrupt;
	ISmess("GSelector::Interrupted select");
	return -1;
      case EINVAL:
	fError = SE_BadArg;
	ISerr("GSelector::Bad n parameter to select");
	return -1;
      case ENOMEM:
	fError = SE_NoMem;
	ISerr("GSelector::No memory for select");
	return -1;
      default:
	fError = SE_Unknown;
	ISerr(GForm("GSelector::Undocumented error in select: %d", errno));
	return -1;
    } // end switch
  }

  if (ret==0) return 0;

  // Build output sets.
  Lock();
  for (GFdSet_i i=fRead.begin(); i!=fRead.end(); ++i)
  {
    if (FD_ISSET(i->second, &read))
      fReadOut.insert(*i);
  }
  for (GFdSet_i i=fWrite.begin(); i!=fWrite.end(); ++i)
  {
    if (FD_ISSET(i->second, &write))
      fWriteOut.insert(*i);
  }
  for (GFdSet_i i=fExcept.begin(); i!=fExcept.end(); i++)
  {
    if (FD_ISSET(i->second, &except))
      fExceptOut.insert(*i);
  }
  Unlock();

  return ret;
}
