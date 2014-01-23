// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GSelector.h"

#include "TSocket.h"

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

GSelector::GSelector(Init_e e) : GMutex(e), fError(SE_Null), fTimeOut(0)
{}

GSelector::~GSelector()
{}

//------------------------------------------------------------------------------

void GSelector::Clear()
{
  Lock();
  fRead.clear();
  fWrite.clear();
  fExcept.clear();
  Unlock();
}

//------------------------------------------------------------------------------

Int_t GSelector::Select()
{
  // Run select on specified sets, with given timeout.
  // Returns number of fds selected or:
  //  0 in case of timeout;
  // -1 in case of error (errno, fError and fErrorStr are set accordingly).

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

  fError = SE_Null; fErrorStr = "";
  errno = 0;
  int ret;
  if (fTimeOut <= 0)
  {
    ret = select(Mfd+1, &read, &write, &except, 0);
  }
  else
  {
    struct timeval timeout;
    timeout.tv_sec  = (time_t) fTimeOut;
    timeout.tv_usec = (time_t)(1000000*(fTimeOut - timeout.tv_sec));
    ret = select(Mfd+1, &read, &write, &except, &timeout);
  }

  if (ret == -1)
  {
    switch (errno)
    {
      case 0: // Cancelled ... or sth ...
	fError = SE_Null;
        fErrorStr = "Unknown error (errno=0).";
	return -1;
      case EBADF:
	fError = SE_BadFD;
	fErrorStr = "Bad file-descriptor.";
	return -1;
      case EINTR:
	fError = SE_Interrupt;
	fErrorStr = "Interrupted select.";
	return -1;
      case EINVAL:
	fError = SE_BadArg;
	fErrorStr = "Bad parameters (num fds or timeout).";
	return -1;
      case ENOMEM:
	fError = SE_NoMem;
	fErrorStr = "No memory for select.";
	return -1;
      default:
	fError = SE_Unknown;
	fErrorStr = GForm("Undocumented error in select, errno=%d.", errno);
	return -1;
    } // end switch
  }

  if (ret == 0) return 0;

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
