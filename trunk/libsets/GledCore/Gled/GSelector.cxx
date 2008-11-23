// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "GSelector.h"

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

ClassImp(GSelector);

/**************************************************************************/

GSelector::GSelector(Init_e e) : GMutex(e) {}

Int_t GSelector::Select()
{
  // prepare sets
  int Mfd=0;
  static fd_set read, write, except;
  FD_ZERO(&read); FD_ZERO(&write); FD_ZERO(&except);
  fReadOut.clear(); fWriteOut.clear(); fExceptOut.clear();
  Lock();
  for(mFdUD_i i=fRead.begin(); i!=fRead.end(); ++i) {
    FD_SET(i->first, &read);
    if(i->first>Mfd) Mfd = i->first;
  }
  for(mFdUD_i i=fWrite.begin(); i!=fWrite.end(); ++i) {
    FD_SET(i->first, &write);
    if(i->first>Mfd) Mfd = i->first;
  }
  for(mFdUD_i i=fExcept.begin(); i!=fExcept.end(); ++i) {
    FD_SET(i->first, &except);
    if(i->first>Mfd) Mfd = i->first;
  }
  Unlock();

  errno = 0;
  int ret;
  if(fTimeOut <= 0) {
    ret = select(Mfd+1, &read, &write, &except, 0);
  } else {
    struct timeval timeout;
    timeout.tv_sec = (time_t) fTimeOut;
    timeout.tv_usec = (time_t)(1000000*(fTimeOut - timeout.tv_sec));
    ret = select(Mfd+1, &read, &write, &except, &timeout);
  }

  if(ret==-1) {
    switch(errno) {
    case 0: // Cancelled ... or sth ...
      fError = SE_Null;
      return -1;
    case EBADF:
      fError = SE_BadFD;
      ISerr("GSelector::Bad FileDescriptor");
      return -1;
    case EINTR:
      fError = SE_Interrupt;
      // ISmess("GSelector::Interrupted select");
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

  if(ret==0) return 0;
  // build output sets
  Lock();
  for(mFdUD_i i=fRead.begin(); i!=fRead.end(); i++) {
    if(FD_ISSET(i->first, &read)) {
      fReadOut.insert(*i);
    }
  }
  for(mFdUD_i i=fWrite.begin(); i!=fWrite.end(); i++) {
    if(FD_ISSET(i->first, &write)) { fWriteOut.insert(*i); }
  }
  for(mFdUD_i i=fExcept.begin(); i!=fExcept.end(); i++) {
    if(FD_ISSET(i->first, &except)) { fExceptOut.insert(*i); }
  }
  Unlock();
  return ret;
}
