// $Id$

// Copyright (C) 1999-2008, Matevz Tadel. All rights reserved.
// This file is part of GLED, released under GNU General Public License version 2.
// For the licensing terms see $GLEDSYS/LICENSE or http://www.gnu.org/.

#include "AEV_NS.h"

#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

const int AEV_NS::BiDiPipe::sRBSize = 1024;

/**************************************************************************/

void AEV_NS::BiDiPipe::init(const TString& base)
{
  static const Exc_t _eh("AEV_NS::BiDiPipe::init ");

  fDir = GForm("/tmp/%s_XXXXXX", base.Data());
  if(mkdtemp((char*) fDir.Data()) == 0) {
    throw(_eh + "mkdtemp failed.");
  }

  fAtoBName = GForm("%s/AtoB", fDir.Data());
  mknod(fAtoBName, 0666 | S_IFIFO, 0);

  fBtoAName = GForm("%s/BtoA", fDir.Data());
  mknod(fBtoAName, 0666 | S_IFIFO, 0);
}

void AEV_NS::BiDiPipe::open_BtoA()
{
  fBtoA = fopen(fBtoAName, "r");
}

void AEV_NS::BiDiPipe::open_AtoB()
{
  fAtoB = fopen(fAtoBName, "w");
  setlinebuf(fAtoB);
}

void AEV_NS::BiDiPipe::close_fifos()
{
  if(fAtoB) { fclose(fAtoB); fAtoB = 0; }
  if(fBtoA) { fclose(fBtoA); fBtoA = 0; }
}

bool AEV_NS::BiDiPipe::eof_BtoA()
{
  return (feof(fBtoA) != 0);
}

/**************************************************************************/

int AEV_NS::BiDiPipe::send_command(const TString& cmd)
{
  // Returns 0 on error, positive int on success (num of records).

  if(bDebug)
  {
    printf("BiDiPipe send_command: %s\n", cmd.Data());
  }

  reset_error();
  fputs(cmd, fAtoB); fputc(10, fAtoB);
  read_line();
  if(strncmp(fRB, "ERR", 3) == 0) {
    bError = true;
    fError = read_string();
    return -1;
  } else {
    return read_int();
  }
}

/**************************************************************************/

TString AEV_NS::BiDiPipe::read_line()
{
  fgets(fRB, sRBSize, fBtoA);
  int s = strlen(fRB);
  // chomp
  if(s && fRB[s-1] == 10) fRB[s-1] = 0;
  if(bDebug) {
    printf("BiDiPipe read_line: %s\n", fRB);
  }
  return fRB;
}

TString AEV_NS::BiDiPipe::read_string()
{
  read_line();
  return fRB;
}

/**************************************************************************/

int AEV_NS::BiDiPipe::read_int()
{
  read_line();
  return (int)strtol(fRB, 0, 0);
}

double AEV_NS::BiDiPipe::read_double()
{
  read_line();
  return strtod(fRB, 0);
}

/**************************************************************************/
